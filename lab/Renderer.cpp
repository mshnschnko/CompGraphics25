#include <string>

#include "Renderer.h"

using namespace DirectX;

Renderer& Renderer::GetInstance() {
    static Renderer rendererInstance;
    return rendererInstance;
}

HRESULT Renderer::InitDevice(const HWND& hWnd) {
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;


    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));

    if (dxgiFactory2)
    {
        hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.BufferCount = 2;

        hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    }

    dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)input.GetWidth();
    viewport.Height = (FLOAT)input.GetHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_pImmediateContext->RSSetViewports(1, &viewport);

    D3D11_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = input.GetWidth();
    rect.bottom = input.GetHeight();
    g_pImmediateContext->RSSetScissorRects(1, &rect);


    g_pRenderedSceneTexture = new RenderTargetTexture(width, height);
    hr = g_pRenderedSceneTexture->initResource(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
        return hr;

    g_pPostProcessedTexture = new RenderTargetTexture(width, height);
    hr = g_pPostProcessedTexture->initResource(g_pd3dDevice, g_pImmediateContext, pBackBuffer);
    if (FAILED(hr))
        return hr;

    pBackBuffer->Release();
    return S_OK;
}

HRESULT Renderer::Init(const HWND& hWnd, const HINSTANCE& hInstance, UINT screenWidth, UINT screenHeight) {
    HRESULT hr = input.InitInputs(hInstance, hWnd, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    hr = camera.Init();
    if (FAILED(hr))
        return hr;

    hr = InitDevice(hWnd);
    if (FAILED(hr))
        return hr;

    hr = scene.Init(g_pd3dDevice, g_pImmediateContext, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    hr = post.Init(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
        return hr;

#ifdef _DEBUG
    hr = g_pImmediateContext->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
    if (FAILED(hr))
        return hr;
#endif

    return S_OK;
}

void Renderer::HandleInput() {
    XMFLOAT3 mouseMove = input.GetMouseInputs();
    camera.MoveByWheel(mouseMove.x, mouseMove.y, mouseMove.z);

    float dx = 0, dy = 0, dz = 0;
    if (input.IsKeyPressed(DIK_W))
        dz += 0.5;
    if (input.IsKeyPressed(DIK_S))
        dz -= 0.5;

    if (input.IsKeyPressed(DIK_A))
        dx -= 0.5;
    if (input.IsKeyPressed(DIK_D))
        dx += 0.5;

    if (input.IsKeyPressed(DIK_LCONTROL))
        dy -= 0.5;
    if (input.IsKeyPressed(DIK_SPACE))
        dy += 0.5;

    camera.MoveByKeyboard(dx, dy, dz);

    scene.ProvideInput(input);
}

bool Renderer::Frame() {
    input.Update();

    HandleInput();
    camera.Frame();
    post.Update(g_pd3dDevice, g_pImmediateContext);

    XMMATRIX mView;
    camera.GetBaseViewMatrix(mView);
    XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)input.GetWidth() / (FLOAT)input.GetHeight(), 0.01f, 100.0f);

    HRESULT hr = scene.Update(g_pImmediateContext, mView, mProjection, camera.GetPos());
    if (FAILED(hr))
        return SUCCEEDED(hr);

    return SUCCEEDED(hr);
}

HRESULT Renderer::Render() {
    g_pImmediateContext->ClearState();
    ID3D11ShaderResourceView* nullSRV = nullptr;
    g_pImmediateContext->PSSetShaderResources(0, 1, &nullSRV);
    g_pRenderedSceneTexture->set(g_pd3dDevice, g_pImmediateContext);

#ifdef _DEBUG
    pAnnotation->BeginEvent((LPCWSTR)(L"Clear background"));
#endif
    g_pRenderedSceneTexture->clear(1.0f, 1.0f, 1.0f, g_pd3dDevice, g_pImmediateContext);
#ifdef _DEBUG
    pAnnotation->EndEvent();
#endif

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)input.GetWidth();
    viewport.Height = (FLOAT)input.GetHeight();
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    g_pImmediateContext->RSSetViewports(1, &viewport);

    D3D11_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = input.GetWidth();
    rect.bottom = input.GetHeight();
    g_pImmediateContext->RSSetScissorRects(1, &rect);

    scene.Render(g_pImmediateContext);

    post.applyTonemapEffect(g_pd3dDevice, g_pImmediateContext, pAnnotation, g_pRenderedSceneTexture, g_pPostProcessedTexture);

    return g_pSwapChain->Present(0, 0);
}

void Renderer::CleanupDevice() {
    post.Release();
    camera.Release();
    input.Release();
    scene.Release();

#ifdef _DEBUG
    if (pAnnotation) pAnnotation->Release();
#endif
    if (g_pPostProcessedTexture)
        delete g_pPostProcessedTexture;

    if (g_pRenderedSceneTexture)
        delete g_pRenderedSceneTexture;

    if (g_pSwapChain1) g_pSwapChain1->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext1) g_pImmediateContext1->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice1) g_pd3dDevice1->Release();

#ifdef _DEBUG
    if (g_pd3dDevice) {
        ID3D11Debug* d3dDebug = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&d3dDebug));

        UINT references = g_pd3dDevice->Release();
        if (references > 1) {
            d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
        d3dDebug->Release();
    }
#else
    if (g_pd3dDevice) g_pd3dDevice->Release();
#endif
}

HRESULT Renderer::ResizeWindow(const HWND& hWnd) {
    if (g_pSwapChain) {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        if ((width != input.GetWidth() || height != input.GetHeight())) {

            if (g_pRenderedSceneTexture)
                g_pRenderedSceneTexture->Release();

            if (g_pPostProcessedTexture)
                g_pPostProcessedTexture->Release();

            HRESULT hr = g_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (SUCCEEDED(hr)) {
                input.Resize(width, height);
                scene.Resize(width, height);
            }

            ID3D11Texture2D* pBackBuffer = nullptr;
            hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
            if (FAILED(hr))
                return hr;

            if (g_pRenderedSceneTexture) {
                g_pRenderedSceneTexture->setScreenSize(width, height);
                hr = g_pRenderedSceneTexture->initResource(g_pd3dDevice, g_pImmediateContext);
                if (FAILED(hr))
                    return hr;

                g_pPostProcessedTexture->setScreenSize(width, height);
                hr = g_pPostProcessedTexture->initResource(g_pd3dDevice, g_pImmediateContext, pBackBuffer);
                if (FAILED(hr))
                    return hr;

                pBackBuffer->Release();
            }
            return SUCCEEDED(hr);
        }
    }
    return S_OK;
}