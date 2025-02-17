cbuffer SceneMatrixBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
    float4 cameraPos;
    int4 lightCount;
    float4 lightPos[10];
    float4 lightColor[10];
    float4 ambientColor;
};