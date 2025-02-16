#include "Light.h"

HRESULT Light::Init(ID3D11Device* device)
{
    std::vector<PointLight> lights = {
    { {1.0f, 2.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, 1.0f, 0.0f },
 // { {2.0f, 2.0f, 2.0f}, {0.0f, 1.0f, 0.0f}, 1.0f, 0.0f },
 // { {0.0f, 2.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, 1.0f, 0.0f }, 
    };
    auto lights_size = static_cast<UINT>(sizeof(PointLight) * lights.size());
    auto lights_p = lights.data();

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = lights_size;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    ZeroMemory(&data, sizeof(data));
    data.pSysMem = lights_p;
    data.SysMemPitch = lights_size;
    data.SysMemSlicePitch = 0;

    return device->CreateBuffer(&bd, &data, &m_pLightsBuffer);
}

ID3D11Buffer* Light::Get()
{
    return m_pLightsBuffer;
}

void Light::Release()
{
    SAFE_RELEASE(m_pLightsBuffer);
}
