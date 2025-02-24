#pragma once
#include <d3d11_1.h>
#include <vector>
#include <memory>
#include <directxmath.h>
#include <chrono>

#include "D3DInclude.h"
#include "screenplane.h"
#include "RenderTargetTexture.h"

class Postprocessing
{
public:

	Postprocessing();

	HRESULT Init(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void Update(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	HRESULT applyTonemapEffect(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		ID3DUserDefinedAnnotation* pAnnotation,
		RenderTargetTexture* inputRTT,
		RenderTargetTexture* resultRTT);

	void Release();
private:
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	void clearScaledHDRTargets();

	void createDownsamplingRTT(
		int width, int height,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void processTexture(
		RenderTargetTexture* inputTex,
		RenderTargetTexture* resultTex,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	ID3D11PixelShader* PSBrightness;
	ID3D11PixelShader* PSCopy;
	ID3D11PixelShader* PSHdr;
	
	// tonemap vars
	ScreenPlane screenPlane;
	ID3D11SamplerState* pSamplerState;
	ID3D11Texture2D* pAverageLumenCPUTexture;
	std::vector<RenderTargetTexture*> scaledHDRTargets;
	
	int maxTextureWidth;
	int maxTextureHeight;

	std::chrono::steady_clock::time_point last; 
	float prevExposure = 0;
	const float eyeAdaptationS = .30f;
	ID3D11Buffer* PSConstantBuffer;

	struct HDRConstantBuffer
	{
		DirectX::XMFLOAT4 averageLumen;
	};
};