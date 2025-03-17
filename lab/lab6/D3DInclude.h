#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <fstream>

class D3DInclude : public ID3DInclude {
public:

  HRESULT __stdcall Open(D3D_INCLUDE_TYPE includeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);

  HRESULT __stdcall Close(LPCVOID pData);

};
