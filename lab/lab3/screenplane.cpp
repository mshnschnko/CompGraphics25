#include <d3dcompiler.h>
#include "D3DInclude.h"
#include "screenplane.h"

HRESULT ScreenPlane::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	D3DInclude includeObj;

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, &includeObj, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}

	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}

HRESULT ScreenPlane::Init(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	// Init shader to draw screen plane with screen texture
	ID3DBlob * pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"screen_plane_VS.hlsl", "main", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
// Create input layout
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


	// Init vertex buffer
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

	// Init index buffer
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

void ScreenPlane::Render(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
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

void ScreenPlane::setVS(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	pContext->VSSetShader(pVertexShader, nullptr, 0u);
	pContext->IASetInputLayout(pProcessTextureLayout);
}
