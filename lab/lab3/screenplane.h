#pragma once
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <vector>

class ScreenPlane
{
public:
	HRESULT Init(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void Render(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void Release();
	
	void setVS(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

private:	
	struct ProcessTextureVertex
	{
		struct
		{
			float x, y, z;
		} pos;

		struct
		{
			UCHAR r, g, b, a;
		} color;

		struct
		{
			float x, y;
		} texcoord;
	};

	const ProcessTextureVertex vertices[4] =
	{
		{-1.f,-1.f,0.f, 0,0,0,1, 0.f, 1.f},
		{ 1.f,-1.f,0.f, 0,0,0,1, 1.f, 1.f},
		{-1.f, 1.f,0.f, 0,0,0,1, 0.f, 0.f},
		{ 1.f, 1.f,0.f, 0,0,0,1, 1.f, 0.f},
	};
	const unsigned short indices[6] = { 0,3,1,3,0,2 };



	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11VertexShader* pVertexShader;
	
	ID3D11InputLayout* pProcessTextureLayout;

};
