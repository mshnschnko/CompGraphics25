#pragma once

#include <d3d11.h>
#include "D3DInclude.h"

// Its here because describes the rule of *object* drawing
enum class IBLMode : int
{
  full = 0,
  diffuse = 1,
  specular = 2,
  nothing = 3,
};


class Rendered {
public:
  virtual HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) = 0;
  virtual void Release() = 0;
  virtual void Render(ID3D11DeviceContext* context) = 0;
  
  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
};
