#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


#include <dinput.h>
#include <directxmath.h>

using namespace DirectX;

class Input {
public:
  // Initialize interface method
  HRESULT InitInputs(const HINSTANCE & g_hInstance, const HWND &hwnd, UINT screenWidth, UINT screenHeight);

  // Update frame method
  bool Update();

  // Release method
  void Release();

  // Useful methods
  XMFLOAT3 IsMouseUsed() const;
  float IsPlusMinusPressed() const;
  bool IsKeyPressed(UCHAR keyCode) const;

  void Resize(UINT screenWidth, UINT screenHeight);
  UINT GetWidth() { return wWidth; }
  UINT GetHeight() { return wHeight; }

private:
  // Read devices methods
  bool ReadKeyboard();
  bool ReadMouse();

  IDirectInput8* directInput = nullptr;
  IDirectInputDevice8* keyboard = nullptr;
  IDirectInputDevice8* mouse = nullptr;

  unsigned char keyboardState[256];
  DIMOUSESTATE mouseState = {};

  UINT wWidth = 0, wHeight = 0;
};
