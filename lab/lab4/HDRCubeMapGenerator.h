#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class HDRCubeMapGenerator {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context);

  HRESULT GenerateCubeMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* txtSRV);

	ID3D11ShaderResourceView* GetSRV() { return g_pCMSRV; };

	void Release();

private:
	void SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight);

	ID3D11PixelShader* g_pPixelShader = nullptr;
	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11SamplerState* g_pSamplerState = nullptr;
	
	ID3D11Texture2D* g_pHDRTexture = nullptr;
	ID3D11RenderTargetView* g_pHDRTextureRTV = nullptr;
	
	ID3D11Texture2D* g_pCubeMapTexture = nullptr;
	ID3D11ShaderResourceView* g_pCMSRV = nullptr;

	struct ConstantBuffer
	{
		XMFLOAT4X4 projectionMatrix;
		XMFLOAT4X4 viewProjectionMatrix;
	};
	ID3D11Buffer* g_pConstantBuffer = nullptr;

	XMMATRIX mProjection;
	XMMATRIX mViews[6];
	XMMATRIX g_mMatrises[6];

	UINT g_hdrTextureSize;
};
