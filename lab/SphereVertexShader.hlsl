cbuffer WorldMatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4 color;
};

cbuffer SceneMatrixBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
};

struct VS_INPUT
{
    float4 position : POSITION;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    output.position = mul(viewProjectionMatrix,
    mul(worldMatrix, input.position)
  );

    return output;
}