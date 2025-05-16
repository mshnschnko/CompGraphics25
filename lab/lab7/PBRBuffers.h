cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  float4 pbr;
  float4 albedo;
};

cbuffer SceneMatrixBuffer : register (b1)
{
  float4x4 viewProjectionMatrix;
  float4 cameraPos;
  int4 lightCount; // x - light count (max 10)
  float4 lightPos[10];
  float4 lightColor[10];
  float4 viewMode;
};

struct VS_INPUT
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
  float2 texUV : TEXCOORD;
};

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
  float2 texUV : TEXCOORD;
};
