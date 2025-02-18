#include "InputHandler.h"

HRESULT InputHandler::InitInputs(const HINSTANCE& g_hInstance, const HWND& hwnd, UINT screenWidth, UINT screenHeight) {
    HRESULT hr = S_OK;

    Resize(screenWidth, screenHeight);

    hr = DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
    if (FAILED(hr))
        return hr;

    hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
    if (FAILED(hr))
        return hr;

    hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr))
        return hr;

    hr = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(hr))
        return hr;

    hr = keyboard->Acquire();
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

void InputHandler::Release() {
    if (mouse) {
        mouse->Unacquire();
        mouse->Release();
        mouse = nullptr;
    }

    if (keyboard) {
        keyboard->Unacquire();
        keyboard->Release();
        keyboard = 0;
    }

    if (directInput) {
        directInput->Release();
        directInput = 0;
    }
}

bool InputHandler::Update() {
    bool result;

    result = ReadKeyboard();
    if (!result)
        return false;

    result = ReadMouse();
    if (!result)
        return false;

    return true;
}

XMFLOAT3 InputHandler::GetMouseInputs() const {
    if (mouseState.rgbButtons[0] || mouseState.rgbButtons[1] || mouseState.rgbButtons[2] & 0x80)
        return XMFLOAT3((float)mouseState.lX, (float)mouseState.lY, (float)mouseState.lZ);
    return XMFLOAT3(0.0f, 0.0f, (float)mouseState.lZ);
};

bool InputHandler::IsKeyPressed(UCHAR keyCode) const {
    return keyboardState[keyCode] & 0x80;
}

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

bool InputHandler::ReadKeyboard() {
    HRESULT result;

    result = keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
    if (FAILED(result)) {
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            if (FAILED(keyboard->Acquire()))
                return false;
            else
                return false;
    }

    return true;
}
