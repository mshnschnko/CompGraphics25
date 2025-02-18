#include <cmath>
#include <directxmath.h>
#include "GeomSphere.h"

void GeomSphere::GenerateSphere(unsigned int LatLines, unsigned int LongLines) {
    using namespace DirectX;

    numSphereVertices = ((LatLines - 2) * LongLines) + 2;
    numSphereFaces = ((LatLines - 3) * (LongLines) * 2) + (LongLines * 2);

    float sphereYaw = 0.0f;
    float spherePitch = 0.0f;

    vertices.resize(numSphereVertices);

    XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    vertices[0].x = 0.0f;
    vertices[0].y = 0.0f;
    vertices[0].z = 1.0f;

    for (size_t i = 0; i < LatLines - 2; i++) {
        spherePitch = (i + 1) * (XM_PI / (LatLines - 1));
        XMMATRIX Rotationx = XMMatrixRotationX(spherePitch);
        for (size_t j = 0; j < LongLines; j++) {
            sphereYaw = j * (XM_2PI / (LongLines));
            XMMATRIX Rotationy = XMMatrixRotationZ(sphereYaw);
            currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
            currVertPos = XMVector3Normalize(currVertPos);
            vertices[i * LongLines + j + 1].x = XMVectorGetX(currVertPos);
            vertices[i * LongLines + j + 1].y = XMVectorGetY(currVertPos);
            vertices[i * LongLines + j + 1].z = XMVectorGetZ(currVertPos);
        }
    }

    vertices[numSphereVertices - 1].x = 0.0f;
    vertices[numSphereVertices - 1].y = 0.0f;
    vertices[numSphereVertices - 1].z = -1.0f;

    indices.resize(numSphereFaces * 3);

    unsigned short k = 0;
    for (unsigned short i = 0; i < LongLines - 1; i++) {
        indices[k] = 0;
        indices[k + 1] = i + 1;
        indices[k + 2] = i + 2;
        k += 3;
    }

    indices[k] = 0;
    indices[k + 1] = LongLines;
    indices[k + 2] = 1;
    k += 3;

    for (size_t i = 0; i < LatLines - 3; i++) {
        for (size_t j = 0; j < LongLines - 1; j++) {
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

    for (size_t i = 0; i < LongLines - 1; i++) {
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

GeomSphere::GeomSphere(float radius, unsigned int LatLines, unsigned intLongLines, float xCenter, float yCenter, float zCenter) noexcept {
    GenerateSphere(10, 10);
    Replace(xCenter, yCenter, zCenter);
}

void GeomSphere::Scale(float scale) noexcept {
    scale = std::abs(scale);

    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].x *= scale;
        vertices[i].y *= scale;
        vertices[i].z *= scale;
    }
}

void GeomSphere::Replace(float x, float y, float z) noexcept {
    sphereCenterX += x;
    sphereCenterY += y;
    sphereCenterZ += z;
}

GeomSphere::~GeomSphere() {
}