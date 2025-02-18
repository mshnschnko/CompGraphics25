#include "CalculateColor.hlsli"

cbuffer WorldMatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4 color;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

float4 main(PS_INPUT input) : SV_Target0
{
    float3 ambient = ambientColor.xyz;

    float3 norm = input.normal;

    return float4(CalculateColor(ambient, norm, input.worldPos.xyz, color.x, false), 1.0);
}