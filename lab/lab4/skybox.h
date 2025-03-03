#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "HDRCubeMapGenerator.h"
#include "IRRGenerator.h"
#include "geomsphere.h"
#include "texture.h"

using namespace DirectX;

class Skybox : public GeomSphere {
public:
  Skybox() {};

  Skybox(const std::wstring& texture_path, unsigned int LatLines = 10, unsigned intLongLines = 10)
    : GeomSphere(LatLines, intLongLines) {
    txt_path = texture_path;
  };

  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void Release();

  void Render(ID3D11DeviceContext* context);

  void Resize(int screenWidth, int screenHeight);

  bool Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

  ID3D11ShaderResourceView* GetIrrSRV() { return irrSRV; }

private:
  struct SBWorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 size;
  };

  struct SBSceneMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 cameraPos;
  };

  float radius = 1.0f;

  // dx11 vars
  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;

  ID3D11InputLayout* g_pVertexLayout = nullptr;
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;

  // Texture with skybox
  HDRCubeMapGenerator hdrCMgen;
  IRRGenerator irrMgen;
  std::wstring txt_path = L".";
  Texture txt;
  ID3D11ShaderResourceView* txtSRV = nullptr;
  ID3D11ShaderResourceView* irrSRV = nullptr;
};
