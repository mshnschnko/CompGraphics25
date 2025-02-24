#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>

#include "D3DInclude.h"
#include "geomSphere.h"
#include "rendered.h"
#include "input.h"

using namespace DirectX;

class Light : public Rendered, GeomSphere {
public:
  Light(XMFLOAT4 color, float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f)
    : GeomSphere(6, 6), color(color), pos(XMFLOAT4(xCenter, yCenter, zCenter, 0.0f)){};

  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void Render(ID3D11DeviceContext* context) {
    if (showSource)
      RenderSphere(context);
  };

  void Release();

  HRESULT Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos);

  void ProvideInput(const Input& input);
  
  XMFLOAT4 GetLightColor() const { return color; };
  XMFLOAT4 GetLightPosition() const { return pos; };

  XMFLOAT4* GetLightColorRef() { return &color; };
  XMFLOAT4* GetLightPositionRef() { return &pos; };

private:
  void RenderSphere(ID3D11DeviceContext* context);

  struct SceneMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
  };

  struct WorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 color;
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

  XMFLOAT4 color;
  XMFLOAT4 pos;
  float radius = 1.0f;
  bool showSource = false;
  float MIN_I = 0.0f, MAX_I = 2048.f;
};
