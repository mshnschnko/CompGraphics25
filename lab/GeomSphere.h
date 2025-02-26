#pragma once

#include <vector>

class GeomSphere {
public:
    GeomSphere(float radius, unsigned int LatLines = 10, unsigned intLongLines = 10,
        float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f) noexcept;

    void Scale(float scale) noexcept;

    void Replace(float x, float y, float z) noexcept;

    void Rotate(float xAngel, float yAngel, float zAngel) noexcept {};

    ~GeomSphere();
protected:
    struct SphereVertex {
        float x, y, z;
    };

    void GenerateSphere(unsigned int LatLines, unsigned int LongLines);

    unsigned int numSphereVertices = 0;
    unsigned int numSphereFaces = 0;
    float radius;
    float sphereCenterX = 0, sphereCenterY = 0, sphereCenterZ = 0;


    unsigned short vertices_cnt;
    std::vector<SphereVertex> vertices;
    std::vector<unsigned short> indices;
};