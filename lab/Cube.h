#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "GeomCube.h"
#include "Light.h"

#define MAX_LIGHT_SOURCES 10

using namespace DirectX;

class Cube : public GeomCube {
private:
    struct Material {
        float shine;
    };

public:
    Cube(Material material, float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f, float scale = 1.0f, float xAngel = 0.0f, float yAngel = 0.0f, float zAngel = 0.0f)
        : GeomCube(xCenter, yCenter, zCenter, scale, xAngel, yAngel, zAngel), cubeMaterial(material) {
    };

    HRESULT Update(ID3D11DeviceContext* context, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, std::vector<Light>& lights);

    HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

    void Render(ID3D11DeviceContext* context);

    void Release();
private:
    struct WorldMatrixBuffer {
        XMMATRIX worldMatrix;
        XMFLOAT4 color;
    };
    struct LightableSceneMatrixBuffer {
        XMMATRIX viewProjectionMatrix;
        XMFLOAT4 cameraPos;
        XMINT4 lightCount;
        XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
        XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
        XMFLOAT4 ambientColor;
    };

    Material cubeMaterial = { 0.0f };

    ID3D11VertexShader* g_pVertexShader = nullptr;
    ID3D11PixelShader* g_pPixelShader = nullptr;
    ID3D11InputLayout* g_pVertexLayout = nullptr;

    ID3D11Buffer* g_pVertexBuffer = nullptr;
    ID3D11Buffer* g_pIndexBuffer = nullptr;
    ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
    ID3D11RasterizerState* g_pRasterizerState = nullptr;
    ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
};