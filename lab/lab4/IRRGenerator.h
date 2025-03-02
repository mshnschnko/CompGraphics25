#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class IRRGenerator {
public:
	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context);

	HRESULT GenerateIrradienceMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV);

	ID3D11ShaderResourceView* GetIRRMapSRV() { return g_pIRRMapSRV; };

	void Release();

private:
	void SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight);

	ID3D11PixelShader* g_pPixelShader = nullptr;
	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11SamplerState* g_pSamplerState = nullptr;

	ID3D11Texture2D* g_pIRRTexture = nullptr;
	ID3D11RenderTargetView* g_pIRRTextureRTV = nullptr;

	ID3D11Texture2D* g_pIRRMap = nullptr;
	ID3D11ShaderResourceView* g_pIRRMapSRV = nullptr;

	struct ConstantBuffer
	{
		XMFLOAT4X4 projectionMatrix;
		XMFLOAT4X4 viewProjectionMatrix;
	};
	ID3D11Buffer* g_pConstantBuffer = nullptr;

	XMMATRIX mProjection;
	XMMATRIX mViews[6];
	XMMATRIX g_mMatrises[6];

	UINT g_irradienceTextureSize;
};
