Texture2D tex : register(t0);
SamplerState smplr : register(s0);

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 localPos : POSITION1;
};


float4 main(PS_INPUT vsout) : SV_TARGET {
    float PI = acos(-1);
    float3 wPos = normalize(vsout.localPos.xyz);

    float2 t_sample = {
        1.0f - atan2(wPos.z, wPos.x) / (2 * PI),
        1.0f - (0.5f + asin(wPos.y) / PI)
    };

    float4 color = tex.Sample(smplr, float3(t_sample, 0));

    return color;
}
