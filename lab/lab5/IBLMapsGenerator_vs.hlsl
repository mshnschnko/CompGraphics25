#include "IBLheader.h"

cbuffer SceneBuffer : register(b0)
{
  float4x4 projectionMatrix;
  float4x4 viewProjectionMatrix;
}

static float4 positions[4] =
{
    { -1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 1.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f }
};

PS_INPUT main(VS_INPUT input) {
  PS_INPUT output;

  output.localPos = mul(positions[input.vertexId], projectionMatrix);
  output.position = mul(output.localPos, viewProjectionMatrix);

  return output;
}