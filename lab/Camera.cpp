#include "Camera.h"

HRESULT Camera::Init() {
    phi = XM_PIDIV4;
    theta = XM_PI/6.0f;
    distanceToPoint = 12.0f;

    pointOfInterest = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    pos = XMVectorAdd(
        XMVectorScale(
            XMVectorSet(
                cosf(theta) * cosf(phi),
                sinf(theta),
                cosf(theta) * sinf(phi), 0.0f
            ), distanceToPoint),
        pointOfInterest);

    float upTheta = theta + XM_PIDIV2;

    up = XMVectorSet(
        cosf(upTheta) * cosf(phi),
        sinf(upTheta),
        cosf(upTheta) * sinf(phi), 0.0f
    );

    Frame();

    return S_OK;
}

void Camera::Frame() {
    pos = XMVectorAdd(
        XMVectorScale(
            XMVectorSet(
                cosf(theta) * cosf(phi),
                sinf(theta),
                cosf(theta) * sinf(phi), 0.0f
            ), distanceToPoint),
        pointOfInterest);

    viewMatrix = DirectX::XMMatrixLookAtLH(pos, pointOfInterest, up);
}

void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix) {
    viewMatrix = this->viewMatrix;
};

void Camera::MoveByWheel(float dx, float dy, float wheel) {
    phi -= dx / movement_downshifting;

    theta += dy / movement_downshifting;
    theta = min(max(theta, -XM_PIDIV2), XM_PIDIV2);

    distanceToPoint -= wheel / movement_downshifting * 5.0f;
    distanceToPoint = max(min(distanceToPoint, 100.0f), 1.0f);
}

void Camera::MoveByKeyboard(float dx, float dy, float dz) {
    XMVECTOR totalVec = XMVectorSet(dx, 0.0f, dz, 0.0f);
    auto viewVec = XMVector4Transform(totalVec, XMMatrixInverse(nullptr, viewMatrix));

    dx = XMVectorGetX(viewVec) * 10 / movement_downshifting;
    dy = dy * 10 / movement_downshifting;
    dz = XMVectorGetZ(viewVec) * 10 / movement_downshifting;

    auto dVec = XMVectorSet(dx, dy, dz, 0.0f);
    pointOfInterest = XMVectorAdd(pointOfInterest, dVec);
    pos = XMVectorAdd(pos, dVec);
}