#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>

class DebugEvents {
public:
  static DebugEvents& GetInstance() {
    static DebugEvents instance;
    return instance;
  };

  HRESULT Init(ID3D11DeviceContext* context) {
    auto hr = context->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
    return hr;
  }

  void beginEvent(const wchar_t* str) {
    if (pAnnotation)
      pAnnotation->BeginEvent((LPCWSTR)(str));
  }

  void endEvent() {
    if (pAnnotation)
      pAnnotation->EndEvent();
  }

  void Release() {
    if (pAnnotation) {
      pAnnotation->Release();
      pAnnotation = nullptr;
    }
  }

private:
  ID3DUserDefinedAnnotation* pAnnotation = nullptr;
};
  

// TODO: Maybe make more 'OOP'?
void beginEvent(const wchar_t* str);

void endEvent();

enum ModelViewMode {
  all = 0,
  normal = 1,
  rm = 2,
  texture = 3
};

struct ViewMode {
  int modelViewMode; // 0 - full, 1 - normal, 2 - roughness/metalness, 3 - texture

  bool isPlainNormal;
  bool isPlainMetalRough;
  bool isPlainColor;
};
