struct VS_INPUT
{
  uint vertexId : SV_VERTEXID;
};

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 localPos : POSITION1;
};
