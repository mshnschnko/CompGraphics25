struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 positionL : TEXCOORD0;
    float3 normal: NORMAL;
    float4 color : COLOR;
};

struct PointLight
{
    float3 Position;
    float3 Color;
    float Attenuation;
};

cbuffer cbLights : register(b0)
{
    PointLight gLights[1];
};

float4 main(PS_INPUT input) : SV_Target0
{
    float3 normal = normalize(input.normal);
    float3 color = input.color;

    for (int i = 0; i < 1; ++i)
    {
        float3 lightDir = gLights[i].Position - input.positionL;
        float distance = length(lightDir);
        lightDir = lightDir / distance;

        float attenuation = 1.0f / (1.0f + gLights[i].Attenuation * distance * distance);

        float diffuse = max(dot(lightDir, normal), 0.0f);
        color += gLights[i].Color * diffuse * attenuation;
    }

    return float4(color, 1.0f);
}