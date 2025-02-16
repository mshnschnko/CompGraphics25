#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <directxmath.h>

#include <ctime>

#include "Camera.h"
#include "InputHandler.h"
#include "Mesh.h"
#include "Light.h"
#include "framework.h"


struct SceneMatrixBuffer {
	XMMATRIX viewProjectionMatrix;
};

class Renderer {
public:
	static Renderer& GetInstance();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;

	HRESULT Init(const HWND& g_hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight);
	HRESULT SetupBackBuffer();

	bool Frame();

	void Render();

	void CleanupDevice();

	void ResizeWindow(const HWND& g_hWnd);

private:
	HRESULT InitDevice(const HWND& g_hWnd);

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	void HandleInput();

	Renderer() = default;

	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11DeviceContext* g_pImmediateContext = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

	ID3D11Texture2D* g_pDepthBuffer = nullptr;
	ID3D11DepthStencilView* g_pDepthBufferDSV = nullptr;

	ID3D11DepthStencilState* g_pDepthState = nullptr;
	ID3D11DepthStencilState* g_pTransDepthState = nullptr;

	ID3D11BlendState* g_pTransBlendState = nullptr;
	ID3D11BlendState* g_pOpaqueBlendState = nullptr;

	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11PixelShader* g_pPixelShader = nullptr;
	ID3D11InputLayout* g_pVertexLayout = nullptr;

	Mesh m_pCube;
	Mesh m_pPlane;
	ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
	ID3D11RasterizerState* g_pRasterizerState = nullptr;

	Light m_pLight;

	std::clock_t init_time;
	Camera camera;
	InputHandler input;

	float angle_velocity = XM_PI;
};