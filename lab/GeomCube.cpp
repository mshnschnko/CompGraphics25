#include <cmath>
#include "GeomCube.h"

GeomCube::GeomCube(float xCenter, float yCenter, float zCenter, float scale, float xAngel, float yAngel, float zAngel) {
    vertices = std::vector<CubeVertex>({
      {{-1, -1,  1}, {0, -1, 0}, {1, 0, 0}},
      {{ 1, -1,  1}, {0, -1, 0}, {1, 0, 0}},
      {{ 1, -1, -1}, {0, -1, 0}, {1, 0, 0}},
      {{-1, -1, -1}, {0, -1, 0}, {1, 0, 0}},

      {{-1,  1, -1}, {0, 1, 0}, {1, 0, 0}},
      {{ 1,  1, -1}, {0, 1, 0}, {1, 0, 0}},
      {{ 1,  1,  1}, {0, 1, 0}, {1, 0, 0}},
      {{-1,  1,  1}, {0, 1, 0}, {1, 0, 0}},

      {{ 1, -1, -1}, {1, 0, 0}, {0, 0, 1}},
      {{ 1, -1,  1}, {1, 0, 0}, {0, 0, 1}},
      {{ 1,  1,  1}, {1, 0, 0}, {0, 0, 1}},
      {{ 1,  1, -1}, {1, 0, 0}, {0, 0, 1}},

      {{-1, -1,  1}, {-1, 0, 0}, {0, 0, -1}},
      {{-1, -1, -1}, {-1, 0, 0}, {0, 0, -1}},
      {{-1,  1, -1}, {-1, 0, 0}, {0, 0, -1}},
      {{-1,  1,  1}, {-1, 0, 0}, {0, 0, -1}},

      {{ 1, -1,  1}, {0, 0, 1}, {-1, 0, 0}},
      {{-1, -1,  1}, {0, 0, 1}, {-1, 0, 0}},
      {{-1,  1,  1}, {0, 0, 1}, {-1, 0, 0}},
      {{ 1,  1,  1}, {0, 0, 1}, {-1, 0, 0}},

      {{-1, -1, -1}, {0, 0, -1}, {1, 0, 0}},
      {{ 1, -1, -1}, {0, 0, -1}, {1, 0, 0}},
      {{ 1,  1, -1}, {0, 0, -1}, {1, 0, 0}},
      {{-1,  1, -1}, {0, 0, -1}, {1, 0, 0}}
        });

    indices = std::vector<unsigned short>({
          0, 2, 1, 0, 3, 2,
          4, 6, 5, 4, 7, 6,
          8, 10, 9, 8, 11, 10,
          12, 14, 13, 12, 15, 14,
          16, 18, 17, 16, 19, 18,
          20, 22, 21, 20, 23, 22
        });

    Scale(scale);
    Replace(xCenter, yCenter, zCenter);
}

void GeomCube::Scale(float scale) {
    scale = std::abs(scale);

    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].pos.x *= scale;
        vertices[i].pos.y *= scale;
        vertices[i].pos.z *= scale;
    }
}

void GeomCube::Replace(float x, float y, float z) noexcept {
    cubeCenterX += x;
    cubeCenterY += y;
    cubeCenterZ += z;

    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].pos.x += x;
        vertices[i].pos.y += y;
        vertices[i].pos.z += z;
    }
}

GeomCube::~GeomCube() {}