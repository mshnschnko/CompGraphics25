// Independet constant buffers for world and view projection matrixes
cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  float4 color;
};

cbuffer SceneMatrixBuffer : register (b1)
{
  float4x4 viewProjectionMatrix;
};

struct VS_INPUT
{
  float3 position : POSITION;
  float3 norm : NORMAL;
};

struct PS_INPUT {
  float4 position : SV_POSITION;
};

PS_INPUT main(VS_INPUT input) {
  PS_INPUT output;

  output.position = mul(viewProjectionMatrix,
    mul(worldMatrix, float4(input.position, 1.0f))
  );

  return output;
}
