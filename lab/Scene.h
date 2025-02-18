#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "InputHandler.h"
#include "Light.h"
#include "Cube.h"

using namespace DirectX;

class Scene {
public:
	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

	void Release();

	void Resize(int screenWidth, int screenHeight);

	void Render(ID3D11DeviceContext* context);

	void ProvideInput(const InputHandler& input);

	HRESULT Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos);

private:
	bool UpdateCubes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos);

#ifdef _DEBUG
	ID3DUserDefinedAnnotation* pAnnotation = nullptr;
#endif

	Cube cube = Cube({ 0.0f });
	std::vector<Light> lights;
};