#include "Mesh.h"

HRESULT Mesh::Init(
    ID3D11Device* device, 
    const std::vector<SimpleVertex> &vertices,
    const std::vector<USHORT> &indices
)
{
    m_pVertexCount = vertices.size();
    auto vertices_size = static_cast<UINT>(sizeof(SimpleVertex) * m_pVertexCount);
    auto p_vertices = vertices.data();

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = vertices_size;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = p_vertices;
    InitData.SysMemPitch = vertices_size;
    InitData.SysMemSlicePitch = 0;

    auto hr = device->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    m_pIndexCount = indices.size();
    auto indices_size = static_cast<UINT>(sizeof(USHORT) * m_pIndexCount);
    auto p_indices = indices.data();

    D3D11_BUFFER_DESC bd1;
    ZeroMemory(&bd1, sizeof(bd1));
    bd1.Usage = D3D11_USAGE_DEFAULT;
    bd1.ByteWidth = indices_size;
    bd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd1.CPUAccessFlags = 0;
    bd1.MiscFlags = 0;
    bd1.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData1;
    ZeroMemory(&InitData1, sizeof(InitData1));
    InitData1.pSysMem = p_indices;
    InitData1.SysMemPitch = indices_size;
    InitData1.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&bd1, &InitData1, &m_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_BUFFER_DESC descWMB = {};
    descWMB.ByteWidth = sizeof(WorldMatrixBuffer);
    descWMB.Usage = D3D11_USAGE_DEFAULT;
    descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    descWMB.CPUAccessFlags = 0;
    descWMB.MiscFlags = 0;
    descWMB.StructureByteStride = 0;

    WorldMatrixBuffer worldMatrixBuffer;
    worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &worldMatrixBuffer;
    data.SysMemPitch = sizeof(worldMatrixBuffer);
    data.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&descWMB, &data, &m_pWorldMatrixBuffer);
    if (FAILED(hr))
        return hr;
}

void Mesh::Render(
    ID3D11DeviceContext* deviceContext,
    ID3D11VertexShader* vertexShader,
    ID3D11PixelShader* pixelShader,
    ID3D11InputLayout* vertexLayout,
    ID3D11Buffer* sceneMatrixBuffer
)
{
    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };
    UINT strides[] = { static_cast<UINT>(sizeof(SimpleVertex))};
    UINT offsets[] = { 0 };
    deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    deviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    deviceContext->IASetInputLayout(vertexLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(vertexShader, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &m_pWorldMatrixBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &sceneMatrixBuffer);
    deviceContext->PSSetShader(pixelShader, nullptr, 0);
    deviceContext->DrawIndexed(m_pIndexCount, 0, 0);
}

void Mesh::Rotate(const DirectX::XMFLOAT3 rotation)
{
}

void Mesh::Translate(const DirectX::XMFLOAT3 translation)
{
}

void Mesh::Release()
{
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pWorldMatrixBuffer);
}
