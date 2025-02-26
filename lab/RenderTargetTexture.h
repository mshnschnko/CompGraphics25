#pragma once

#include <vector>
#include <d3d11_1.h>

class RenderTargetTexture
{
public:
	RenderTargetTexture(int width, int height);

	HRESULT initResource(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		ID3D11Resource* pBackBuffer = nullptr);

	void set(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext) const;

	void setAsResource(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext) const;

	void clear(
		float red, float green, float blue,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void copyToTexture(
		ID3D11Texture2D* target,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext) const;

	void setScreenSize(int width, int height) {
		this->width = width;
		this->height = height;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
	}

	void Release();

	~RenderTargetTexture();

private:
	int width, height;

	ID3D11Texture2D* pTexture2D;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11ShaderResourceView* pShaderResourceView;

	D3D11_VIEWPORT vp;
};
