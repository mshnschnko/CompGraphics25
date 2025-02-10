#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <directxmath.h>

using namespace DirectX;

class InputHandler {
public:
	HRESULT InitInputs(const HINSTANCE& g_hInstance, const HWND& hwnd, UINT screenWidth, UINT screenHeight);

	void Realese();

	XMFLOAT3 GetMouseInputs() const;

	void Resize(UINT screenWidth, UINT screenHeight);
	UINT GetWidth() { return wWidth; }
	UINT GetHeight() { return wHeight; }
	bool ReadMouse();

private:

	IDirectInput8* directInput = nullptr;
	IDirectInputDevice8* mouse = nullptr;

	DIMOUSESTATE mouseState = {};

	UINT wWidth = 0, wHeight = 0;
};