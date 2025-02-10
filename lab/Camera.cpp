#include "Camera.h"

HRESULT Camera::Init() {
    phi = XM_PIDIV4;
    theta = XM_PI/6.0f;
    distanceToPoint = 4.0f;

    pointOfInterest = XMFLOAT3(0.0f, 0.0f, 0.0f);

    return S_OK;
}

void Camera::Frame() {
    XMFLOAT3 pos = XMFLOAT3(
        cosf(theta) * cosf(phi),
        sinf(theta),
        cosf(theta) * sinf(phi)
    );

    pos.x = pos.x * distanceToPoint + pointOfInterest.x;
    pos.y = pos.y * distanceToPoint + pointOfInterest.y;
    pos.z = pos.z * distanceToPoint + pointOfInterest.z;

    float upTheta = theta + XM_PIDIV2;
    XMFLOAT3 up = XMFLOAT3(
        cosf(upTheta) * cosf(phi),
        sinf(upTheta),
        cosf(upTheta) * sinf(phi)
    );

    viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f),
        DirectX::XMVectorSet(pointOfInterest.x, pointOfInterest.y, pointOfInterest.z, 0.0f),
        DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f)
    );
}

void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix) {
    viewMatrix = this->viewMatrix;
};

void Camera::Move(float dx, float dy, float wheel) {
    phi -= dx / movement_downshifting;

    theta += dy / movement_downshifting;
    theta = min(max(theta, -XM_PIDIV2), XM_PIDIV2);

    distanceToPoint -= wheel / movement_downshifting;
    distanceToPoint = max(distanceToPoint, 1.0f);
}