#include <cmath>
#include <directxmath.h>
#include "geomSphere.h"

void GeomSphere::GenerateSphere(unsigned int LatLines, unsigned int LongLines) {
  using namespace DirectX;

  // generate verticies  
  numSphereVertices = ((LatLines - 2) * LongLines) + 2;
  numSphereFaces = ((LatLines - 3) * (LongLines) * 2) + (LongLines * 2);

  float sphereYaw = 0.0f;
  float spherePitch = 0.0f;

  vertices.resize(numSphereVertices);

  XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

  vertices[0].pos.x = vertices[0].norm.x = 0.0f;
  vertices[0].pos.y = vertices[0].norm.y = 0.0f;
  vertices[0].pos.z = vertices[0].norm.z = 1.0f;

  for (unsigned int i = 0; i < LatLines - 2; i++) {
    spherePitch = (i + 1) * (XM_PI / (LatLines - 1));
    XMMATRIX Rotationx = XMMatrixRotationX(spherePitch);
    for (unsigned int j = 0; j < LongLines; j++) {
      sphereYaw = j * (XM_2PI / (LongLines));
      XMMATRIX Rotationy = XMMatrixRotationZ(sphereYaw);
      currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
      currVertPos = XMVector3Normalize(currVertPos);
      vertices[i * LongLines + j + 1].pos.x = vertices[i * LongLines + j + 1].norm.x = XMVectorGetX(currVertPos);
      vertices[i * LongLines + j + 1].pos.y = vertices[i * LongLines + j + 1].norm.y = XMVectorGetY(currVertPos);
      vertices[i * LongLines + j + 1].pos.z = vertices[i * LongLines + j + 1].norm.z = XMVectorGetZ(currVertPos);
    }
  }
  
  vertices[numSphereVertices - 1].pos.x = vertices[numSphereVertices - 1].norm.x = 0.0f;
  vertices[numSphereVertices - 1].pos.y = vertices[numSphereVertices - 1].norm.y = 0.0f;
  vertices[numSphereVertices - 1].pos.z = vertices[numSphereVertices - 1].norm.z = -1.0f;

  // generate indicies
  indices.resize(numSphereFaces * 3);

  unsigned int k = 0;
  for (unsigned int i = 0; i < LongLines - 1; i++) {
    indices[k] = 0;
    indices[k + 1] = i + 1;
    indices[k + 2] = i + 2;
    k += 3;
  }

  indices[k] = 0;
  indices[k + 1] = LongLines;
  indices[k + 2] = 1;
  k += 3;

  for (unsigned int i = 0; i < LatLines - 3; i++) {
    for (unsigned int j = 0; j < LongLines - 1; j++) {
      indices[k] = i * LongLines + j + 1;
      indices[k + 1] = i * LongLines + j + 2;
      indices[k + 2] = (i + 1) * LongLines + j + 1;

      indices[k + 3] = (i + 1) * LongLines + j + 1;
      indices[k + 4] = i * LongLines + j + 2;
      indices[k + 5] = (i + 1) * LongLines + j + 2;

      k += 6;
    }

    indices[k] = (i * LongLines) + LongLines;
    indices[k + 1] = (i * LongLines) + 1;
    indices[k + 2] = ((i + 1) * LongLines) + LongLines;

    indices[k + 3] = ((i + 1) * LongLines) + LongLines;
    indices[k + 4] = (i * LongLines) + 1;
    indices[k + 5] = ((i + 1) * LongLines) + 1;

    k += 6;
  }

  for (unsigned int i = 0; i < LongLines - 1; i++) {
    indices[k] = numSphereVertices - 1;
    indices[k + 1] = (numSphereVertices - 1) - (i + 1);
    indices[k + 2] = (numSphereVertices - 1) - (i + 2);
    k += 3;
  }

  indices[k] = numSphereVertices - 1;
  indices[k + 1] = (numSphereVertices - 1) - LongLines;
  indices[k + 2] = numSphereVertices - 2;

  return;
}

GeomSphere::GeomSphere(unsigned int LatLines, unsigned int LongLines) noexcept {
  GenerateSphere(LatLines, LongLines);
}

GeomSphere::~GeomSphere() {
}
