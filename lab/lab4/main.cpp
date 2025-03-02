#include <windows.h>
#include <xstring>
#include <mmsystem.h>

#include "resource1.h"
#include "renderer.h"

#define START_W 1280
#define START_H 720

#define MAX_LOADSTRING 300
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text


// Global Variables
HINSTANCE       g_hInst = nullptr;
HWND            g_hWnd = nullptr;


// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


// Register class and create window
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
  // Register class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = nullptr;
  wcex.hIconSm = nullptr;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = L"WindowClass";
  if (!RegisterClassEx(&wcex))
    return E_FAIL;

  // Create window
  g_hInst = hInstance;
  RECT rc = { 0, 0, START_W, START_H };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  g_hWnd = CreateWindow(L"WindowClass", L"Lab4 5040102/40201",
    WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
    nullptr);
  if (!g_hWnd)
    return E_FAIL;

  ShowWindow(g_hWnd, nCmdShow);
  SetForegroundWindow(g_hWnd);
  SetFocus(g_hWnd);

  return S_OK;
}

// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (FAILED(InitWindow(hInstance, nCmdShow)))
      return 0;

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);


    // Fix working folder
    std::wstring dir;
    dir.resize(MAX_LOADSTRING + 1);
    GetCurrentDirectory(MAX_LOADSTRING + 1, &dir[0]);
    size_t configPos = dir.find(L"x64");
    if (configPos != std::wstring::npos)
    {
      dir.resize(configPos);
      dir += szTitle;
      SetCurrentDirectory(dir.c_str());
    }

    // Init Device
    auto hr = Renderer::GetInstance().Init(g_hWnd, g_hInst, START_W, START_H);
    if (FAILED(hr))
    {
        Renderer::GetInstance().CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (Renderer::GetInstance().Update())
            if (FAILED(Renderer::GetInstance().Render()))
                break;
    }

    Renderer::GetInstance().CleanupDevice();

    return (int)msg.wParam;
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Called every time the application receives a message
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
      return true;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;


    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 256;
        lpMMI->ptMinTrackSize.y = 256;
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        if (FAILED(Renderer::GetInstance().ResizeWindow(g_hWnd)))
            PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

