#include "SceneCB.hlsli"

float3 CalculateColor(in float3 objColor, in float3 objNormal, in float3 pos, in float shine, in bool trans)
{
    float3 finalColor = float3(0, 0, 0);

    for (int i = 0; i < lightCount.x; i++)
    {
        float3 norm = objNormal;

        float3 lightDir = lightPos[i].xyz - pos;
        float lightDist = length(lightDir);
        lightDir /= lightDist;

        float atten = clamp(1.0 / (lightDist * lightDist), 0, 1);

        if (trans && dot(lightDir, objNormal) < 0.0)
        {
            norm = -norm;
        }
        finalColor += objColor * max(dot(lightDir, norm), 0) * atten * lightColor[i].xyz * lightColor[i].w;
    }

    return finalColor;
}