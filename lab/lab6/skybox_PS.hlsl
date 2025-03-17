TextureCube tex : register (t0);
SamplerState smplr : register (s0);

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float3 localPos : POSITION1;
};

float4 main(PS_INPUT input) : SV_Target0{
  return float4(
      tex.Sample(smplr, input.localPos).xyz,
      1.0
    );
}
