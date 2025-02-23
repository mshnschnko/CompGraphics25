#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <directxmath.h>

#include <ctime>

#include "Camera.h"
#include "InputHandler.h"
#include "Scene.h"
#include "RenderTargetTexture.h"
#include "Postprocessing.h"
#include "framework.h"


class Renderer {
public:
	static Renderer& GetInstance();
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;

	HRESULT Init(const HWND& g_hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight);

	bool Frame();

	HRESULT Render();

	void CleanupDevice();

	HRESULT ResizeWindow(const HWND& g_hWnd);

private:
	HRESULT InitDevice(const HWND& g_hWnd);

	void HandleInput();

	Renderer() = default;

	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11Device1* g_pd3dDevice1 = nullptr;
	ID3D11DeviceContext* g_pImmediateContext = nullptr;
	ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	IDXGISwapChain1* g_pSwapChain1 = nullptr;
	RenderTargetTexture* g_pRenderedSceneTexture;
	RenderTargetTexture* g_pPostProcessedTexture;

	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11PixelShader* g_pPixelShader = nullptr;
	ID3D11InputLayout* g_pVertexLayout = nullptr;

	ID3D11Buffer* g_pVertexBuffer = nullptr;
	ID3D11Buffer* g_pIndexBuffer = nullptr;
	ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
	ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
	ID3D11RasterizerState* g_pRasterizerState = nullptr;
	ID3DUserDefinedAnnotation* pAnnotation = nullptr;

	Camera camera;
	InputHandler input;
	Scene scene;
	Postprocessing post;
};