#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>

#include "skybox.h"
#include "materials.h"
#include "geomSphere.h"
#include "D3DInclude.h"
#include "input.h"
#include "light.h"

#define MAX_LIGHT_SOURCES 10

using namespace DirectX;

class Sphere : public GeomSphere {
public:
  Sphere() {};

  Sphere(float radius, XMFLOAT3 pos, Skybox& sb, XMFLOAT3 albedo, float roughness, float metalness,
    unsigned int LatLines = 10, unsigned intLongLines = 10)
    : GeomSphere(LatLines, intLongLines), pos(pos), radius(radius),
    pbrMaterial(albedo, roughness, metalness)
  {
    irrSRV = sb.GetIrrSRV();
  };

  HRESULT Update(ID3D11DeviceContext* context, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, const std::vector<Light>& lights, const PBRMaterial& material, const PBRMode& mode);

  void ProvideInput(const Input& input);

  XMFLOAT4 GetPosition() const { return XMFLOAT4(pos.x, pos.y, pos.z, 1.f); };

  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void SetIrrMapSRV(ID3D11ShaderResourceView* newIrrSRV) {
    irrSRV = newIrrSRV;
  };

  void Release();

  void Render(ID3D11DeviceContext* context);
protected:
  struct SceneMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 cameraPos;
    XMINT4 lightCount;
    XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
    XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
    XMFLOAT4 ambientColor;
  };

  struct WorldMatrixBuffer {
    XMMATRIX worldMatrix;
    PBRMaterial pbrMaterial;
    PBRMode pbrMode;
  };

  // dx11 vars
  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;

  ID3D11InputLayout* g_pVertexLayout = nullptr;
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;

  // var for outer resources (no need to release them)
  ID3D11ShaderResourceView* irrSRV = nullptr;

  // Sphere object params
  PBRMaterial pbrMaterial;
  PBRMode pbrMode = PBRMode::allPBR;
  float radius;
  XMFLOAT3 pos;
};
