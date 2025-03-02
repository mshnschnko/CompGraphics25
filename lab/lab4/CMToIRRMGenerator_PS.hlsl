TextureCube tex : register(t0);
SamplerState smplr : register(s0);

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 localPos : POSITION1;
};


float4 main(PS_INPUT vsout) : SV_TARGET{
    float3 normal = normalize(vsout.localPos.xyz);
    float3 dir = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(dir, normal));
    float3 binormal = cross(normal, tangent);

    float4 irradiance = float4(0.0, 0.0, 0.0, 0.0);
    const int N1 = 200;
    const int N2 = 50;
    float PI = acos(-1);

    for (int i = 0; i < N1; ++i)
    {
        for (int j = 0; j < N2; ++j)
        {
            float phi = i * (2.0f * PI / N1);
            float theta = j * (PI / 2.0f / N2);
            float3 a = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 t_sample = a.x * tangent + a.y * binormal + a.z * normal;

            irradiance += tex.Sample(smplr, t_sample) * cos(theta) * sin(theta);
        }
    }

    irradiance = PI * irradiance / (N1 * N2);

    return irradiance;
}
