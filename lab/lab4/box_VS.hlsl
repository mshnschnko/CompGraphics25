#include "sceneConstantBuffer.hlsli"

// Independet constant buffers for world and view projection matrixes
cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  float4 color; // x - specular power
};

struct VS_INPUT
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
};

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
};

PS_INPUT main(VS_INPUT input) {
  PS_INPUT output;
  
  output.worldPos = mul(worldMatrix, float4(input.position, 1.0f));
  output.position = mul(viewProjectionMatrix, output.worldPos);
  output.normal = mul(worldMatrix, input.normal);
  output.tangent = mul(worldMatrix, input.tangent);
  
  return output;
}
