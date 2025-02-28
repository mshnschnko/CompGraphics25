#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <vector>

#include "DDSTextureLoader.h"

class Texture {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
  // TODO: make more params in Ex initializing version
  HRESULT InitEx(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
  
  HRESULT InitArray(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<const wchar_t*>& filenames);
  
  void Release();

  ID3D11ShaderResourceView* GetTexture();
private:
  ID3D11ShaderResourceView* g_pTextureView = nullptr;
};
