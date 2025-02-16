struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal: NORMAL;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target0
{
    return input.color;
}