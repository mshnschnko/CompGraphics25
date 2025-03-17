#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include "input.h"

using namespace DirectX;

#define MOVEMENT_DOWNSHIFTING 300.f

class Camera {
public:
  // Initialization camera method
  HRESULT InitCamera();

  // Update frame method
  void Update();

  // Release method
  void Release() {};

  XMVECTOR GetPos() const { return pos; };

  // Get view matrix method
  void GetBaseViewMatrix(XMMATRIX& viewMatrix);

  void ProvideInput(const Input& input);
private:
  // Rotate camera with mouse method around point of interest
  void Rotate(float dx, float dy, float wheel);

  // Move camera with keyboards pressed buttons
  void Move(float dx, float dy, float dz);

  XMMATRIX viewMatrix;

  XMVECTOR pointOfInterest;
  XMVECTOR pos;
  XMVECTOR up;
};