#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct IBLMaps {
	ID3D11ShaderResourceView* pIRRMapSRV = nullptr;
	ID3D11ShaderResourceView* pPrefilMapSRV = nullptr;
	ID3D11ShaderResourceView* pBRDFMapSRV = nullptr;
};

class IBLMapsGenerator {
public:
	IBLMapsGenerator() {
		N1 = 300;
		N2 = 300;
		g_irradienceTextureSize = 32;
		g_prefilTextureSize = 128;
		g_BRDFTextureSize = 128;
		g_prefilMipMapLevels = 5;
		InitMatricies();
	};

	IBLMapsGenerator(UINT irradienceTextureSize, UINT irrN1, UINT irrN2, UINT prefilTextureSize, UINT BRDFTextureSize, UINT prefilMipMapLevels) {
		N1 = irrN1;
		N2 = irrN2;
		g_irradienceTextureSize = irradienceTextureSize;
		g_prefilTextureSize = prefilTextureSize;
		g_BRDFTextureSize = BRDFTextureSize;
		g_prefilMipMapLevels = prefilMipMapLevels;
		InitMatricies();
	}

	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context);

	HRESULT GenerateMaps(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV);

	IBLMaps GetMaps() {
		IBLMaps maps = {};
		maps.pIRRMapSRV = g_pIRRMapSRV;
		maps.pPrefilMapSRV = g_pPrefilMapSRV;
		maps.pBRDFMapSRV = g_pBRDFMapSRV;
		return maps;
	}

	void Release();

private:
	void InitMatricies();

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	HRESULT GenerateIrranienceMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV);
	HRESULT GeneratePrefilteredMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV);
	HRESULT GenerateBRDF(ID3D11Device* device, ID3D11DeviceContext* context);

	void SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight);

	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11SamplerState* g_pSamplerState = nullptr;

	// Vars for generating Irradience map
	ID3D11PixelShader* g_pIrrCMPixelShader = nullptr;
	//  - for texture and RTV where to draw sides
	ID3D11Texture2D* g_pIRRTexture = nullptr;
	ID3D11RenderTargetView* g_pIRRTextureRTV = nullptr;
	//  - for cubeMap and its' ShaderResourceView where to draw sides
	ID3D11Texture2D* g_pIRRMap = nullptr;
	ID3D11ShaderResourceView* g_pIRRMapSRV = nullptr;

	// Vars for generating prefiltered color
	ID3D11PixelShader* g_pPrefilPixelShader = nullptr;
	//  - for texture and RTV where to draw sides
	ID3D11Texture2D* g_pPrefilTexture = nullptr;
	ID3D11RenderTargetView* g_pPrefilTextureRTV = nullptr;
	//  - for maps for diff roughness and its' ShaderResourceView where to draw
	ID3D11Texture2D* g_pPrefilMap = nullptr;
	ID3D11ShaderResourceView* g_pPrefilMapSRV = nullptr;

	// Vars for generating preintegrated BRDF color
	ID3D11PixelShader* g_pBRDFPixelShader = nullptr;
	//  - for texture and RTV where to draw sides
	ID3D11Texture2D* g_pBRDFTexture = nullptr;
	ID3D11RenderTargetView* g_pBRDFTextureRTV = nullptr;
	//  - not actually a 'map' - just a texture for shader resource, not RTV
	ID3D11Texture2D* g_pBRDFMap = nullptr;
	ID3D11ShaderResourceView* g_pBRDFMapSRV = nullptr;

	struct ConstantBuffer
	{
		XMFLOAT4X4 projectionMatrix;
		XMFLOAT4X4 viewProjectionMatrix;
	};
	ID3D11Buffer* g_pConstantBuffer = nullptr;

	struct PrefilConstantBuffer
	{
		XMFLOAT4 roughness; // roughness.r - roughness
	};
	ID3D11Buffer* g_pPrefilConstantBuffer = nullptr;

	struct IRRConstantBuffer
	{
		XMINT4 param; // roughness.r - roughness
	};
	ID3D11Buffer* g_pIRRConstantBuffer = nullptr;


	XMMATRIX mProjection;
	XMMATRIX mViews[6];
	XMMATRIX g_mMatrises[6];

	INT N1 = 200, N2 = 200;

	UINT g_irradienceTextureSize = 32;
	UINT g_prefilTextureSize = 128;
	UINT g_BRDFTextureSize = 128;
	UINT g_prefilMipMapLevels = 5;
};
