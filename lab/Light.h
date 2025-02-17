#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>

#include "Sphere.h"
#include "InputHandler.h"

using namespace DirectX;

class Light {
public:
    Light(XMFLOAT4 color, float radius, unsigned int LatLines = 10, unsigned intLongLines = 10, float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f)
        : sp(color, radius, LatLines, intLongLines, xCenter, yCenter, zCenter) {
    };

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
        return sp.Init(device, context, screenWidth, screenHeight);
    }

    void Render(ID3D11DeviceContext* context) {
        if (showSource)
            sp.Render(context);
    };

    void Release() {
        sp.Release();
    };

    void Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
        sp.Update(context, viewMatrix, projectionMatrix, cameraPos);
    };

    void ProvideInput(const InputHandler& input);

    XMFLOAT4 GetLightColor() { return sp.GetColor(); };
    XMFLOAT4 GetLightPosition() { return sp.GetPosition(); };

private:
    Sphere sp;
    bool showSource = false;
    float MIN_I = 0.0f, MAX_I = 2048.f;
};