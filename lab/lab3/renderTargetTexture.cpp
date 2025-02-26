#include "renderTargetTexture.h"

RenderTargetTexture::RenderTargetTexture(int width, int height) : width(width), height(height) {
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
}

HRESULT RenderTargetTexture::initResource(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	ID3D11DepthStencilView* _depthStencilView,
	ID3D11Resource* pBackBuffer)
{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	
	HRESULT hr = S_OK;
	if (pBackBuffer)
	{
		hr = pDevice->CreateTexture2D(&td, nullptr, &pTexture2D);
		if (FAILED(hr))
			return hr;

		hr = pDevice->CreateShaderResourceView(pTexture2D, nullptr, &pShaderResourceView);
		if (FAILED(hr))
			return hr; 
		
		hr = pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		hr = pDevice->CreateTexture2D(&td, nullptr, &pTexture2D);
		if (FAILED(hr))
			return hr; 
		
		hr = pDevice->CreateShaderResourceView(pTexture2D, nullptr, &pShaderResourceView);
		if (FAILED(hr))
			return hr; 
		
		hr = pDevice->CreateRenderTargetView(pTexture2D, nullptr, &pRenderTargetView);
		if (FAILED(hr))
			return hr;
	}

	depthStencilView = _depthStencilView;
	return S_OK;
}

void RenderTargetTexture::set(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext) const
{
	pContext->OMSetRenderTargets(1, &pRenderTargetView, depthStencilView);
	pContext->RSSetViewports(1u, &vp);
}

void RenderTargetTexture::setAsResource(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext) const {
	pContext->PSSetShaderResources(0u, 1u, &pShaderResourceView);
}

void RenderTargetTexture::clear(
	float red, float green, float blue,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext) {
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pRenderTargetView, color);
}

void RenderTargetTexture::copyToTexture(
	ID3D11Texture2D* target,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext) const {
	pContext->CopyResource(target, pTexture2D);
}

void RenderTargetTexture::Release() {
	if (pShaderResourceView) pShaderResourceView->Release();
	if (pRenderTargetView) pRenderTargetView->Release();
	if (pTexture2D) pTexture2D->Release();
}

RenderTargetTexture::~RenderTargetTexture() {
	Release();
}
