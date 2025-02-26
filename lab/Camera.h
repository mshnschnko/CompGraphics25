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

	XMVECTOR GetPos() const { return pos; };

	void MoveByWheel(float dx, float dy, float wheel);
	void MoveByKeyboard(float dx, float dy, float dz);

private:
	XMMATRIX viewMatrix;
	XMVECTOR pointOfInterest;
	XMVECTOR pos;
	XMVECTOR up;

	float distanceToPoint;
	float phi;
	float theta;
};