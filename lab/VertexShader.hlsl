cbuffer WorldMatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
};

cbuffer SceneMatrixBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal: NORMAL;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPos = mul(worldMatrix, float4(input.position, 1.0));
    output.position = mul(viewProjectionMatrix, worldPos);
    output.color = input.color;
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    return output;
}