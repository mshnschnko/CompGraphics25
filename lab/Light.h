#pragma once
#include "framework.h"
#include <d3d11.h>
#include <DirectXMath.h>

struct PointLight {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
	float Attenuation;
	float _padding;
};

class Light
{
public:
	Light() : m_pLightsBuffer(nullptr) {};
	HRESULT Init(ID3D11Device* device);
	ID3D11Buffer* Get();
	void Release();
private:
	ID3D11Buffer* m_pLightsBuffer;
};