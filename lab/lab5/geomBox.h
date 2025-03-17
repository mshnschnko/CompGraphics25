#pragma once

#include <vector>

class GeomBox {
public:
  GeomBox();

protected:
  struct BoxVertex
  {
    struct {
      float x, y, z;
    } pos;       // positional coords
    struct {
      float x, y, z;
    }  normal;    // normal vec
    struct {
      float x, y, z;
    }  tangent;   // tangent vec
  };

  std::vector<BoxVertex> vertices;
  std::vector<unsigned short> indices;
};
