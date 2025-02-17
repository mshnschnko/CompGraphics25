cbuffer WorldMatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4 color;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return color;
}