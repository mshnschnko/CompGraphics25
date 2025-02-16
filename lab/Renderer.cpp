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

	if (SUCCEEDED(hr))
	{
		hr = SetupBackBuffer();
	}

	// Create blend states
	if (SUCCEEDED(hr))
	{
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		hr = g_pd3dDevice->CreateBlendState(&desc, &g_pTransBlendState);
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			desc.RenderTarget[0].BlendEnable = FALSE;
			hr = g_pd3dDevice->CreateBlendState(&desc, &g_pOpaqueBlendState);
		}
		assert(SUCCEEDED(hr));
	}

	// Create reverse depth state
	if (SUCCEEDED(hr))
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = TRUE; // Enable depth testing
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // Allow writing to the depth buffer
		desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		desc.StencilEnable = FALSE;
		hr = g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthState);
		assert(SUCCEEDED(hr));
	}

	// Create reverse transparent depth state
	if (SUCCEEDED(hr))
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		desc.StencilEnable = FALSE;
		hr = g_pd3dDevice->CreateDepthStencilState(&desc, &g_pTransDepthState);
		assert(SUCCEEDED(hr));
	}

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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

	hr = CreateCubeMesh(g_pd3dDevice, m_pCube, RGB(52, 81, 29));
	if (FAILED(hr)) {
		return hr;
	}
	m_pCube.Translate({ 0.0f, 2.0f, 0.0f });
	m_pCube.Update(g_pImmediateContext);

	hr = CreatePlaneMesh(g_pd3dDevice, m_pPlane, RGB(0, 0, 0));
	if (FAILED(hr)) {
		return hr;
	}
	m_pPlane.Translate({ -2.5f, 0, -2.5f });
	m_pPlane.Update(g_pImmediateContext);

	hr = m_pLight.Init(g_pd3dDevice);
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
	descRastr.FrontCounterClockwise = true;
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

HRESULT Renderer::SetupBackBuffer()
{
	ID3D11Texture2D* pBackBuffer = NULL;
	HRESULT result = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (SUCCEEDED(result))
	{
		result = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);

		SAFE_RELEASE(pBackBuffer);
	}
	if (SUCCEEDED(result))
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Height = input.GetHeight();
		desc.Width = input.GetWidth();
		desc.MipLevels = 1;

		result = g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_pDepthBuffer);
		assert(SUCCEEDED(result));
	}
	if (SUCCEEDED(result))
	{
		result = g_pd3dDevice->CreateDepthStencilView(g_pDepthBuffer, nullptr, &g_pDepthBufferDSV);
		assert(SUCCEEDED(result));
	}

	assert(SUCCEEDED(result));

	return result;
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

	auto duration = static_cast<float>((1.0 * clock() - init_time) / CLOCKS_PER_SEC);
	m_pCube.Rotate({0.0f, duration * angle_velocity, 0.0f});
	m_pCube.Update(g_pImmediateContext);

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

	ID3D11RenderTargetView* views[] = { g_pRenderTargetView };;
	g_pImmediateContext->OMSetRenderTargets(1, views, g_pDepthBufferDSV);

	static const FLOAT BackColor[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, BackColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthBufferDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

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

	g_pImmediateContext->OMSetDepthStencilState(g_pDepthState, 1);

	g_pImmediateContext->RSSetState(g_pRasterizerState);

	g_pImmediateContext->OMSetBlendState(g_pOpaqueBlendState, nullptr, 0xFFFFFFFF);

	m_pPlane.Render(
		g_pImmediateContext, g_pVertexShader, g_pPixelShader, g_pVertexLayout, g_pSceneMatrixBuffer, m_pLight.Get()
	);

	m_pCube.Render(
		g_pImmediateContext, g_pVertexShader, g_pPixelShader, g_pVertexLayout, g_pSceneMatrixBuffer, m_pLight.Get()
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
	m_pLight.Release();

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