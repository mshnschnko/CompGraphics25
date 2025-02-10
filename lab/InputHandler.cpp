#include "InputHandler.h"

HRESULT InputHandler::InitInputs(const HINSTANCE& g_hInstance, const HWND& hwnd, UINT screenWidth, UINT screenHeight) {
    HRESULT hr = S_OK;

    Resize(screenWidth, screenHeight);

    hr = DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
    if (FAILED(hr))
        return hr;

    hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
    if (FAILED(hr))
        return hr;

    hr = mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(hr))
        return hr;

    hr = mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr))
        return hr;

    hr = mouse->Acquire();
    if (FAILED(hr))
        return hr;

    return hr;

}

void InputHandler::Realese() {
    if (mouse) {
        mouse->Unacquire();
        mouse->Release();
        mouse = nullptr;
    }

    if (directInput) {
        directInput->Release();
        directInput = 0;
    }
}

XMFLOAT3 InputHandler::GetMouseInputs() const {
    if (mouseState.rgbButtons[0] || mouseState.rgbButtons[1] || mouseState.rgbButtons[2] & 0x80)
        return XMFLOAT3((float)mouseState.lX, (float)mouseState.lY, (float)mouseState.lZ);
    return XMFLOAT3(0.0f, 0.0f, (float)mouseState.lZ);
};

void InputHandler::Resize(UINT screenWidth, UINT screenHeight) {
    wWidth = screenWidth;
    wHeight = screenHeight;
}

bool InputHandler::ReadMouse() {
    HRESULT result;

    result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
    if (FAILED(result)) {
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            mouse->Acquire();
        else
            return false;
    }

    return true;
}