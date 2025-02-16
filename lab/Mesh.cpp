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
    ID3D11Buffer* sceneMatrixBuffer,
    ID3D11Buffer* lightsBuffer
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
    deviceContext->PSSetConstantBuffers(0, 1, &lightsBuffer);
    deviceContext->PSSetShader(pixelShader, nullptr, 0);
    deviceContext->DrawIndexed(m_pIndexCount, 0, 0);
}

void Mesh::Rotate(const DirectX::XMFLOAT3 rotation)
{
    m_pRotation.x = std::fmodf(rotation.x, DirectX::XM_2PI);
    m_pRotation.y = std::fmodf(rotation.y, DirectX::XM_2PI);
    m_pRotation.z = std::fmodf(rotation.z, DirectX::XM_2PI);

    if (m_pRotation.x < 0) {
        m_pRotation.x += DirectX::XM_2PI;
    }
    if (m_pRotation.y < 0) {
        m_pRotation.y += DirectX::XM_2PI;
    }
    if (m_pRotation.z < 0) {
        m_pRotation.z += DirectX::XM_2PI;
    }
}

void Mesh::Translate(const DirectX::XMFLOAT3 translation)
{
    m_pPosition.x += translation.x;
    m_pPosition.y += translation.y;
    m_pPosition.z += translation.z;
}

void Mesh::Update(ID3D11DeviceContext* context)
{
    auto translationMatrix = DirectX::XMMatrixTranslation(m_pPosition.x, m_pPosition.y, m_pPosition.z);
    auto rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_pRotation.x, m_pRotation.y, m_pRotation.z);
    WorldMatrixBuffer worldMatrixBuffer;
    worldMatrixBuffer.worldMatrix = rotationMatrix * translationMatrix;
    context->UpdateSubresource(m_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);
}

void Mesh::Release()
{
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pWorldMatrixBuffer);
}

HRESULT CreateCubeMesh(ID3D11Device* device, Mesh &mesh, COLORREF color)
{
    std::vector<SimpleVertex> vertices = {
        // Front face
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, color }, // 0
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, color }, // 1
        { {0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, color }, // 2
        { {0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, color }, // 3

        // Back face
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, color },  // 4
        { {0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, color },  // 5
        { {0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, color },  // 6
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, color },  // 7

        // Left face
        { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, color }, // 8
        { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, color }, // 9
        { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, color }, // 10
        { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, color }, // 11

        // Right face
        { {0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, color },  // 12
        { {0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, color },  // 13
        { {0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, color },  // 14
        { {0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, color },  // 15

        // Top face
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, color },  // 16
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, color },  // 17
        { {0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, color },  // 18
        { {0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, color },  // 19

        // Bottom face
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, color }, // 20
        { {0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, color }, // 21
        { {0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, color }, // 22
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, color }  // 23
    };
    std::vector<USHORT> indices = {
        // Front face
        0, 1, 2,  // Triangle 1
        0, 2, 3,  // Triangle 2

        // Back face
        4, 5, 6,  // Triangle 1
        4, 6, 7,  // Triangle 2

        // Left face
        8, 9, 10, // Triangle 1
        8, 10, 11,// Triangle 2

        // Right face
        12, 13, 14, // Triangle 1
        12, 14, 15, // Triangle 2

        // Top face
        16, 17, 18, // Triangle 1
        16, 18, 19, // Triangle 2

        // Bottom face
        20, 21, 22, // Triangle 1
        20, 22, 23  // Triangle 2
    };
    return mesh.Init(device, vertices, indices);
}

HRESULT CreatePlaneMesh(ID3D11Device* device, Mesh& mesh, COLORREF color)
{
    const USHORT size = 5;
    std::vector<SimpleVertex> vertices;
    vertices.reserve((size + 1) * (size + 1));
    for (USHORT i = 0; i < size + 1; i++) {
        for (USHORT j = 0; j < size + 1; j++) {
            SimpleVertex v = { 
                {static_cast<float>(i), 0.0f, static_cast<float>(j)},
                {0.0f, 1.0f, 0.0f},
                color
            };
            vertices.push_back(v);
        }
    }

    std::vector<USHORT> indices;
    indices.reserve(size * size * 6);
    for (USHORT i = 0; i < size; i++) {
        for (USHORT j = 0; j < size; j++) {
            USHORT topLeft = i * (size + 1) + j;
            USHORT bottomLeft = (i + 1) * (size + 1) + j;
            USHORT topRight = i * (size + 1) + (j + 1);
            USHORT bottomRight = (i + 1) * (size + 1) + (j + 1);

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
            indices.push_back(topRight);
        }
    }
    return mesh.Init(device, vertices, indices);
}
