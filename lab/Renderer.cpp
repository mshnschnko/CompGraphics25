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

	IDXGIFactory* pFactory = nullptr;
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

	// Select hardware adapter
	IDXGIAdapter* pSelectedAdapter = NULL;
	if (SUCCEEDED(hr))
	{
		IDXGIAdapter* pAdapter = NULL;
		UINT adapterIdx = 0;
		while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
		{
			DXGI_ADAPTER_DESC desc;
			pAdapter->GetDesc(&desc);

			if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0)
			{
				pSelectedAdapter = pAdapter;
				break;
			}

			pAdapter->Release();

			adapterIdx++;
		}
	}
	assert(pSelectedAdapter != NULL);

	// Create DirectX 11 device
	D3D_FEATURE_LEVEL level;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
	if (SUCCEEDED(hr))
	{
		UINT flags = 0;
#ifdef _DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
		hr = D3D11CreateDevice(pSelectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
			flags, levels, 1, D3D11_SDK_VERSION, &g_pd3dDevice, &level, &g_pImmediateContext);
		assert(level == D3D_FEATURE_LEVEL_11_0);
		assert(SUCCEEDED(hr));
	}

	// Create swapchain
	if (SUCCEEDED(hr))
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = g_hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = true;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		hr = pFactory->CreateSwapChain(g_pd3dDevice, &swapChainDesc, &g_pSwapChain);
		assert(SUCCEEDED(hr));
	}

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

	hr = CreateCubeMesh(g_pd3dDevice, m_pCube);
	if (FAILED(hr)) {
		return hr;
	}

	hr = CreatePlaneMesh(g_pd3dDevice, m_pPlane, RGB(0, 0, 0));
	if (FAILED(hr)) {
		return hr;
	}

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

	// WorldMatrixBuffer worldMatrixBuffer;

	// worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity);

	// g_pImmediateContext->UpdateSubresource(g_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

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

	m_pCube.Render(
		g_pImmediateContext, g_pVertexShader, g_pPixelShader, g_pVertexLayout, g_pSceneMatrixBuffer
	);

	m_pPlane.Render(
		g_pImmediateContext, g_pVertexShader, g_pPixelShader, g_pVertexLayout, g_pSceneMatrixBuffer
	);

	g_pSwapChain->Present(0, 0);

	if (pAnnotation) {
		pAnnotation->EndEvent();
		pAnnotation->Release();
	}
}

void Renderer::CleanupDevice() {
	m_pCube.Release();
	m_pPlane.Release();

	camera.Release();
	input.Realese();
	g_pImmediateContext->ClearState();

	if (g_pRasterizerState) g_pRasterizerState->Release();
	if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
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