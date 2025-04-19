#include "IBLheader.h"

cbuffer PrefilConstantbuffer : register(b0)
{
  int4 params;  // N1 = params.x, N2 = params.y, 
};

TextureCube tex : register(t0);
SamplerState smplr : register(s0);

float4 main(PS_INPUT input) : SV_TARGET{
    float3 normal = normalize(input.localPos.xyz);
    float3 dir = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(dir, normal));
    float3 binormal = cross(normal, tangent);

    float3 irradiance = float3(0.0, 0.0, 0.0);
    int N1 = params.x;
    int N2 = params.y;
    float PI = acos(-1);

    for (int i = 0; i < N1; i++)
    {
        for (int j = 0; j < N2; j++)
        {
            float phi = i * (2.0f * PI / N1);
            float theta = j * (PI / 2.0f / N2);
            float3 a = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 t_sample = a.x * tangent + a.y * binormal + a.z * normal;

            irradiance += tex.Sample(smplr, t_sample) * cos(theta) * sin(theta);
        }
    }

    irradiance = PI * irradiance / (N1 * N2);

    return float4(irradiance, 0.0);
}
