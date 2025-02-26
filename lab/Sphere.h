#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>

#include "GeomSphere.h"
#include "InputHandler.h"

using namespace DirectX;

class Sphere : public GeomSphere {
public:
    Sphere(XMFLOAT4 color, float radius, unsigned int LatLines = 10, unsigned intLongLines = 10, float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f)
        : color(color), GeomSphere(radius, LatLines, intLongLines, xCenter, yCenter, zCenter) {
    };

    bool Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos);


    void SetWColor(float newW) { color.w = newW; };
    XMFLOAT4 GetColor() { return color; };
    XMFLOAT4 GetPosition() { return XMFLOAT4(sphereCenterX, sphereCenterY, sphereCenterZ, 1.f); };

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

    void Release();

    void Render(ID3D11DeviceContext* context);
protected:
    struct SceneMatrixBuffer {
        XMMATRIX viewProjectionMatrix;
    };

    struct WorldMatrixBuffer {
        XMMATRIX worldMatrix;
        XMFLOAT4 color;
    };

    ID3D11Buffer* g_pVertexBuffer = nullptr;
    ID3D11Buffer* g_pIndexBuffer = nullptr;
    ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
    ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
    ID3D11RasterizerState* g_pRasterizerState = nullptr;

    ID3D11InputLayout* g_pVertexLayout = nullptr;
    ID3D11VertexShader* g_pVertexShader = nullptr;
    ID3D11PixelShader* g_pPixelShader = nullptr;

    XMFLOAT4 color;
};