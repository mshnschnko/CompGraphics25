#pragma once

#include <vector>

class GeomCube {
public:
    GeomCube(float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f, float scale = 1.0f, float xAngel = 0.0f, float yAngel = 0.0f, float zAngel = 0.0f);

    void Scale(float scale);

    void Replace(float x, float y, float z) noexcept;

    ~GeomCube() noexcept;
protected:
    struct CubeVertex
    {
        struct {
            float x, y, z;
        } pos;
        struct {
            float x, y, z;
        }  normal;
        struct {
            float x, y, z;
        }  tangent;
    };

    std::vector<CubeVertex> vertices;
    std::vector<unsigned short> indices;

    float cubeCenterX = 0, cubeCenterY = 0, cubeCenterZ = 0;
};