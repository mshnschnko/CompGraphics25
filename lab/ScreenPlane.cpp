#include <d3dcompiler.h>
#include "screenplane.h"

HRESULT ScreenPlane::Init(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = D3DReadFileToBlob(L"ScreenPlaneVertexShader.cso", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"The ScreenPlaneVertexShader.cso file not found.", L"Error", MB_OK);
		return hr;
	}

	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
	const D3D11_INPUT_ELEMENT_DESC inputDescCopy[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Color",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Texcoord",0,DXGI_FORMAT_R32G32_FLOAT,0,16u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};

	hr = pDevice->CreateInputLayout(
		inputDescCopy, (UINT)std::size(inputDescCopy),
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&pProcessTextureLayout
	);
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(ProcessTextureVertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = vertices;

	hr = pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;

	hr = pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

void ScreenPlane::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {
	const UINT stride = sizeof(ProcessTextureVertex);
	const UINT offset = 0u;

	pContext->IASetVertexBuffers(0u, 1u, &pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0u);
	pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
}

void ScreenPlane::Release() {
	if (pIndexBuffer) pIndexBuffer->Release();
	if (pVertexBuffer) pVertexBuffer->Release();
	if (pProcessTextureLayout) pProcessTextureLayout->Release();
	if (pVertexShader) pVertexShader->Release();
}

void ScreenPlane::setVS(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {
	pContext->VSSetShader(pVertexShader, nullptr, 0u);
	pContext->IASetInputLayout(pProcessTextureLayout);
}