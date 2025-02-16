#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

constexpr float movement_downshifting = 300.f;

class Camera {
public:
	HRESULT Init();

	void Release() {};

	void Frame();

	void GetBaseViewMatrix(XMMATRIX& viewMatrix);

	void Move(float dx, float dy, float wheel);

private:
	XMMATRIX viewMatrix;
	XMFLOAT3 pointOfInterest;

	float distanceToPoint;
	float phi;
	float theta;
};