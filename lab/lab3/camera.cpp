#include "camera.h"

// Initialization camera method
HRESULT Camera::InitCamera() {
  float phi = 0/*XM_PIDIV2 * -0.8f*/;
  float theta = 0/*XM_PIDIV2 * 0.8f*/;
  float distanceToPoint = 10.0f;
  pointOfInterest = XMVectorSet(-5.0f, 0.0f, 0.0f, 1.0f);

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

  Update();

  return S_OK;
}

// Update frame method
void Camera::Update() {
  viewMatrix = DirectX::XMMatrixLookAtLH(pos, pointOfInterest, up);
}

void Camera::ProvideInput(const Input& input) {
  // handle camera rotations
  XMFLOAT3 mouseMove = input.IsMouseUsed();
  Rotate(mouseMove.x * 4 / MOVEMENT_DOWNSHIFTING, mouseMove.y * 4 / MOVEMENT_DOWNSHIFTING, mouseMove.z * 4 / MOVEMENT_DOWNSHIFTING);

  float dx = 0, dz = 0;
  if (input.IsKeyPressed(DIK_W))  // forward
    dz += 1;
  if (input.IsKeyPressed(DIK_S))  // backward
    dz -= 1;

  if (input.IsKeyPressed(DIK_A))  // right
    dx -= 1;
  if (input.IsKeyPressed(DIK_D))  // left
    dx += 1;

  XMVECTOR totalVec = XMVectorSet(dx, 0.0f, dz, 0.0f);
  auto viewVec = XMVector4Transform(totalVec, XMMatrixInverse(nullptr, viewMatrix));

  Move(XMVectorGetX(viewVec) * 30 / MOVEMENT_DOWNSHIFTING, 0, XMVectorGetZ(viewVec) * 30 / MOVEMENT_DOWNSHIFTING);
}

void Camera::Rotate(float dx, float dy, float wheel) {
  auto dVec = XMVectorSet(
    -(float)dx,
    (float)dy,
    0.f,
    0.f);
  auto viewDVec = XMVector4Transform(dVec, XMMatrixInverse(nullptr, viewMatrix));
  pointOfInterest = XMVectorAdd(pointOfInterest, viewDVec);
}


void Camera::Move(float dx, float dy, float dz) {
  auto dVec = XMVectorSet(dx, dy, dz, 0.0f);
  pointOfInterest = XMVectorAdd(pointOfInterest, dVec);
  pos = XMVectorAdd(pos, dVec);
}

// Get view matrix method
void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix) {
  viewMatrix = this->viewMatrix;
};
