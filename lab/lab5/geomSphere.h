#pragma once

#include <vector>

using namespace DirectX;

class GeomSphere {
public:
  GeomSphere() {};

  GeomSphere(unsigned int LatLines, unsigned int LongLines) noexcept;

  ~GeomSphere();
protected:
  // TODO: make uniform structure of vertexes for all geom primitives
  struct SphereVertex
  {
    XMFLOAT3 pos;          // positional coords
    XMFLOAT3 norm;         // normal vec
  };

  void GenerateSphere(unsigned int LatLines, unsigned int LongLines);

  // Sphere light geometry params
  unsigned int numSphereVertices = 0;
  unsigned int numSphereFaces = 0;
  
  unsigned int vertices_cnt = 0;
  std::vector<SphereVertex> vertices;
  std::vector<unsigned int> indices;
};