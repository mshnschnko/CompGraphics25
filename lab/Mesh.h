#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "framework.h"

struct SimpleVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	COLORREF color;
};

struct WorldMatrixBuffer {
	DirectX::XMMATRIX worldMatrix;
};

class Mesh
{
public:
	Mesh() = default;
	HRESULT Init(
		ID3D11Device* device,
		const std::vector<SimpleVertex>& vertices,
		const std::vector<USHORT>& indices
	);
	void Render(
		ID3D11DeviceContext* deviceContext,
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader,
		ID3D11InputLayout* vertexLayout,
		ID3D11Buffer* sceneMatrixBuffer
	);
	void Rotate(const DirectX::XMFLOAT3 rotation);
	void Translate(const DirectX::XMFLOAT3 translation);
	void Update(ID3D11DeviceContext* context);
	void Release();
private:
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	ID3D11Buffer* m_pWorldMatrixBuffer = nullptr;

	size_t m_pVertexCount;
	size_t m_pIndexCount;

	DirectX::XMFLOAT3 m_pPosition = { 0, 0, 0 };
	DirectX::XMFLOAT3 m_pRotation = { 0, 0, 0 };
};

HRESULT CreateCubeMesh(ID3D11Device* device, Mesh &mesh, COLORREF color);
HRESULT CreatePlaneMesh(ID3D11Device* device, Mesh& mesh, COLORREF color);