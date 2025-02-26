#include <d3dcompiler.h>
#include <cmath>

#include "Postprocessing.h"

Postprocessing::Postprocessing() : maxTextureHeight(0), maxTextureWidth(0) {}

void Postprocessing::Update(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	D3D11_VIEWPORT vp = { 0 };
	unsigned vpNum = 1;
	pContext->RSGetViewports(&vpNum, &vp);
	if (vp.Width != maxTextureWidth || vp.Height != maxTextureHeight)
	{
		clearScaledHDRTargets();
		createDownsamplingRTT((int)vp.Width, (int)vp.Height, pDevice, pContext);
	}

	for (auto& rtt : scaledHDRTargets)
		rtt->clear(1.f, 1.f, 1.f, pDevice, pContext);
}

HRESULT Postprocessing::Init(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	ID3DBlob* pPSBlob = nullptr;
	HRESULT hr = D3DReadFileToBlob(L"Brightness.cso", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Brightness.cso file not found.", L"Error", MB_OK);
		return hr;
	}

	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &PSBrightness);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;


	pPSBlob = nullptr;
	hr = D3DReadFileToBlob(L"SamplingPixelShader.cso", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The SamplingPixelShader.cso file not found.", L"Error", MB_OK);
		return hr;
	}

	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &PSCopy);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	pPSBlob = nullptr;
	hr = D3DReadFileToBlob(L"HDR.cso", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The HDR.cso file not found.", L"Error", MB_OK);
		return hr;
	}

	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &PSHdr);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = 1;
	td.Height = 1;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_STAGING;
	td.BindFlags = 0;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = pDevice->CreateTexture2D(&td, nullptr, &pAverageLumenCPUTexture);
	if (FAILED(hr))
		return hr;

	D3D11_VIEWPORT vp = { 0 };
	unsigned vpNum = 1;
	pContext->RSGetViewports(&vpNum, &vp);
	createDownsamplingRTT((int)vp.Width, (int)vp.Height, pDevice, pContext);

	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	hr = pDevice->CreateSamplerState(&sd, &pSamplerState);
	if (FAILED(hr))
		return hr;

	hr = screenPlane.Init(pDevice, pContext);
	if (FAILED(hr))
		return hr;

	const HDRConstantBuffer hdrcb = { { prevExposure, 0.f, 0.f, 0.f } };
	D3D11_BUFFER_DESC hdrcbDesc = { 0 };
	hdrcbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hdrcbDesc.Usage = D3D11_USAGE_DYNAMIC;
	hdrcbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hdrcbDesc.MiscFlags = 0u;
	hdrcbDesc.ByteWidth = sizeof(hdrcb);
	hdrcbDesc.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA hdrsd = {};
	hdrsd.pSysMem = &hdrcb;

	hr = pDevice->CreateBuffer(&hdrcbDesc, &hdrsd, &PSConstantBuffer);

	return hr;
}

void Postprocessing::createDownsamplingRTT(
	int width, int height,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	maxTextureHeight = height;
	maxTextureWidth = width;
	int rtv_num = static_cast<int>(std::floor(std::log2(width < height ? width : height)));
	RenderTargetTexture* rtt = new RenderTargetTexture(height, width);
	rtt->initResource(pDevice, pContext);
	scaledHDRTargets.push_back(rtt);

	for (size_t i = 0; i <= rtv_num; ++i) {
		int dim = (1 << (rtv_num - i));
		rtt = new RenderTargetTexture(dim, dim);
		rtt->initResource(pDevice, pContext);
		scaledHDRTargets.push_back(rtt);
	}
}

HRESULT Postprocessing::applyTonemapEffect(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	ID3DUserDefinedAnnotation* pAnnotation,
	RenderTargetTexture* inputRTT,
	RenderTargetTexture* resultRTT)
{
#ifdef _DEBUG
	pAnnotation->BeginEvent(L"Average Brightness");
#endif

	pContext->PSSetShader(PSBrightness, nullptr, 0u);
	processTexture(inputRTT, scaledHDRTargets[0], pDevice, pContext);

	pContext->PSSetShader(PSCopy, nullptr, 0u);
	for (size_t i = 1; i < scaledHDRTargets.size(); i++)
		processTexture(scaledHDRTargets[i - 1], scaledHDRTargets[i], pDevice, pContext);

	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	resultRTT->set(pDevice, pContext);

	D3D11_MAPPED_SUBRESOURCE averageTextureData;
	ZeroMemory(&averageTextureData, sizeof(averageTextureData));
	scaledHDRTargets.back()->copyToTexture(pAverageLumenCPUTexture, pDevice, pContext);

	HRESULT hr = pContext->Map(pAverageLumenCPUTexture, 0, D3D11_MAP_READ, 0, &averageTextureData);
	float averageLogBrightness = std::exp(*(float*)averageTextureData.pData) - 1.0f;
	pContext->Unmap(pAverageLumenCPUTexture, 0u);

	const auto old = last;
	last = std::chrono::steady_clock::now();
	float duration = std::chrono::duration<float>(last - old).count();

	float expGain = (1 - std::exp(-duration / eyeAdaptationS));
	prevExposure += (averageLogBrightness - prevExposure) * expGain;

#ifdef _DEBUG
	pAnnotation->EndEvent();
#endif

#ifdef _DEBUG
	pAnnotation->BeginEvent(L"Tonemaping");
#endif
	pContext->PSSetShader(PSHdr, nullptr, 0u);

	D3D11_MAPPED_SUBRESOURCE subresource;
	hr = pContext->Map(PSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	if (FAILED(hr))
		return FAILED(hr);

	HDRConstantBuffer& sceneBuffer = *reinterpret_cast<HDRConstantBuffer*>(subresource.pData);
	sceneBuffer.averageLumen = DirectX::XMFLOAT4(prevExposure, 0.f, 0.f, 0.f);
	pContext->Unmap(PSConstantBuffer, 0);

	pContext->PSSetConstantBuffers(0u, 1u, &PSConstantBuffer);
	processTexture(inputRTT, resultRTT, pDevice, pContext);

#ifdef _DEBUG
	pAnnotation->EndEvent();
#endif

	return hr;
}

void Postprocessing::processTexture(
	RenderTargetTexture* inputTex,
	RenderTargetTexture* resultTex,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	ID3D11ShaderResourceView* const pSRV[1] = { nullptr };

	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	inputTex->setAsResource(pDevice, pContext);
	resultTex->set(pDevice, pContext);
	pContext->PSSetSamplers(0, 1, &pSamplerState);

	screenPlane.setVS(pDevice, pContext);
	screenPlane.Render(pDevice, pContext);
}

void Postprocessing::Release() {
	clearScaledHDRTargets();

	if (PSConstantBuffer) PSConstantBuffer->Release();

	screenPlane.Release();

	if (pSamplerState) pSamplerState->Release();

	if (pAverageLumenCPUTexture) pAverageLumenCPUTexture->Release();

	if (PSHdr) PSHdr->Release();
	if (PSCopy) PSCopy->Release();
	if (PSBrightness) PSBrightness->Release();
}

void Postprocessing::clearScaledHDRTargets() {
	for (size_t i = 0; i < scaledHDRTargets.size(); i++) {
		delete scaledHDRTargets[i];
	}
	scaledHDRTargets.clear();
}