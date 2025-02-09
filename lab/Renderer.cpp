#include <d3d11sdklayers.h>
#include <string>

#include "Renderer.h"

using namespace DirectX;

Renderer& Renderer::GetInstance() {
    static Renderer rendererInstance;
    return rendererInstance;
}

HRESULT Renderer::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Renderer::InitDevice(const HWND& g_hWnd) {
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
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

        hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
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
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    }

    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    ID3DBlob* pVSBlob = nullptr;
    hr = D3DReadFileToBlob(L"VertexShader.cso", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"The VertexShader.cso file not found.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    const char* vsName = "Vertex Shader";
    g_pVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(vsName)), vsName);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    UINT numElements = ARRAYSIZE(layout);

    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    const char* vertexLayoutName = "Input Layout";
    g_pVertexLayout->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(vertexLayoutName)), vertexLayoutName);

    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    ID3DBlob* pPSBlob = nullptr;
    hr = D3DReadFileToBlob(L"PixelShader.cso", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The PixelShader.cso file not found.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    const char* psName = "Pixel Shader";
    g_pPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(psName)), psName);

    init_time = clock();

    SimpleVertex vertices[] = {
         { -1.0f, 1.0f, -1.0f, RGB(0, 0, 255) },
         { 1.0f, 1.0f, -1.0f, RGB(0, 255, 0) },
         { 1.0f, 1.0f, 1.0f, RGB(0, 255, 255) },
         { -1.0f, 1.0f, 1.0f, RGB(255, 0, 0) },
         { -1.0f, -1.0f, -1.0f, RGB(255, 0, 255) },
         { 1.0f, -1.0f, -1.0f, RGB(255, 255, 0) },
         { 1.0f, -1.0f, 1.0f, RGB(255, 255, 255) },
         { -1.0f, -1.0f, 1.0f, RGB(0, 0, 0) }
    };

    USHORT indices[] = {
          3,1,0,
          2,1,3,

          0,5,4,
          1,5,0,

          3,4,7,
          0,4,3,

          1,6,5,
          2,6,1,

          2,7,6,
          3,7,2,

          6,4,5,
          7,4,6,
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &vertices;
    InitData.SysMemPitch = sizeof(vertices);
    InitData.SysMemSlicePitch = 0;

    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC bd1;
    ZeroMemory(&bd1, sizeof(bd1));
    bd1.Usage = D3D11_USAGE_DEFAULT;
    bd1.ByteWidth = sizeof(indices);
    bd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd1.CPUAccessFlags = 0;
    bd1.MiscFlags = 0;
    bd1.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData1;
    ZeroMemory(&InitData1, sizeof(InitData1));
    InitData1.pSysMem = &indices;
    InitData1.SysMemPitch = sizeof(indices);
    InitData1.SysMemSlicePitch = 0;

    hr = g_pd3dDevice->CreateBuffer(&bd1, &InitData1, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC descWMB = {};
    descWMB.ByteWidth = sizeof(WorldMatrixBuffer);
    descWMB.Usage = D3D11_USAGE_DEFAULT;
    descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    descWMB.CPUAccessFlags = 0;
    descWMB.MiscFlags = 0;
    descWMB.StructureByteStride = 0;

    WorldMatrixBuffer worldMatrixBuffer;
    worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &worldMatrixBuffer;
    data.SysMemPitch = sizeof(worldMatrixBuffer);
    data.SysMemSlicePitch = 0;

    hr = g_pd3dDevice->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC descSMB = {};
    descSMB.ByteWidth = sizeof(SceneMatrixBuffer);
    descSMB.Usage = D3D11_USAGE_DYNAMIC;
    descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    descSMB.MiscFlags = 0;
    descSMB.StructureByteStride = 0;

    hr = g_pd3dDevice->CreateBuffer(&descSMB, nullptr, &g_pSceneMatrixBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_RASTERIZER_DESC descRastr = {};
    descRastr.AntialiasedLineEnable = false;
    descRastr.FillMode = D3D11_FILL_SOLID;
    descRastr.CullMode = D3D11_CULL_BACK;
    descRastr.DepthBias = 0;
    descRastr.DepthBiasClamp = 0.0f;
    descRastr.FrontCounterClockwise = false;
    descRastr.DepthClipEnable = true;
    descRastr.ScissorEnable = false;
    descRastr.MultisampleEnable = false;
    descRastr.SlopeScaledDepthBias = 0.0f;

    hr = g_pd3dDevice->CreateRasterizerState(&descRastr, &g_pRasterizerState);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT Renderer::Init(const HWND& g_hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight) {
    HRESULT hr = input.InitInputs(g_hInstance, g_hWnd, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    hr = camera.Init();
    if (FAILED(hr))
        return hr;

    hr = InitDevice(g_hWnd);
    if (FAILED(hr))
        return hr;
    
    return S_OK;
}

void Renderer::HandleInput() {
    XMFLOAT3 mouseMove = input.GetMouseInputs();
    camera.Move(mouseMove.x, mouseMove.y, mouseMove.z);
}

bool Renderer::Frame() {
    ID3DUserDefinedAnnotation* pAnnotation = nullptr;
    if (SUCCEEDED(g_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&pAnnotation)))) {
        pAnnotation->BeginEvent(L"Frame Calculation");
    }

    input.ReadMouse();

    HandleInput();
    camera.Frame();

    auto duration = (1.0 * clock() - init_time) / CLOCKS_PER_SEC;

    WorldMatrixBuffer worldMatrixBuffer;

    worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity);

    g_pImmediateContext->UpdateSubresource(g_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

    XMMATRIX mView;
    camera.GetBaseViewMatrix(mView);

    XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)input.GetWidth() / (FLOAT)input.GetHeight(), 0.01f, 100.0f);

    D3D11_MAPPED_SUBRESOURCE subresource;
    HRESULT hr = g_pImmediateContext->Map(g_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    if (FAILED(hr)) {
        if (pAnnotation) {
            pAnnotation->EndEvent();
            pAnnotation->Release();
        }
        return FAILED(hr);
    }

    SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
    sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(mView, mProjection);
    g_pImmediateContext->Unmap(g_pSceneMatrixBuffer, 0);

    if (pAnnotation) {
        pAnnotation->EndEvent();
        pAnnotation->Release();
    }

    return SUCCEEDED(hr);
}

void Renderer::Render() {
    ID3DUserDefinedAnnotation* pAnnotation = nullptr;
    if (SUCCEEDED(g_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&pAnnotation)))) {
        pAnnotation->BeginEvent(L"Rendering");
    }

    g_pImmediateContext->ClearState();

    ID3D11RenderTargetView* views[] = { g_pRenderTargetView };
    g_pImmediateContext->OMSetRenderTargets(1, views, nullptr);

    float ClearColor[4] = { (float)0.19, (float)0.84, (float)0.78, (float)1.0 };

    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

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

    g_pImmediateContext->RSSetState(g_pRasterizerState);

    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffer };
    UINT strides[] = { 16 };
    UINT offsets[] = { 0 };
    g_pImmediateContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);
    g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->DrawIndexed(36, 0, 0);

    g_pSwapChain->Present(0, 0);

    if (pAnnotation) {
        pAnnotation->EndEvent();
        pAnnotation->Release();
    }
}

void Renderer::CleanupDevice() {
    camera.Realese();
    input.Realese();
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pRasterizerState) g_pRasterizerState->Release();
    if (g_pWorldMatrixBuffer) g_pWorldMatrixBuffer->Release();
    if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain1) g_pSwapChain1->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext1) g_pImmediateContext1->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice1) g_pd3dDevice1->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

void Renderer::ResizeWindow(const HWND& g_hWnd) {
    if (g_pSwapChain)
    {
        g_pImmediateContext->OMSetRenderTargets(0, 0, 0);

        g_pRenderTargetView->Release();

        HRESULT hr;
        hr = g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        ID3D11Texture2D* pBuffer;
        hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
            (void**)&pBuffer);

        hr = g_pd3dDevice->CreateRenderTargetView(pBuffer, NULL,
            &g_pRenderTargetView);

        pBuffer->Release();

        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_pImmediateContext->RSSetViewports(1, &vp);

        input.Resize(width, height);
    }
}