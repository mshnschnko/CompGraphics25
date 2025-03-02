struct PBRMaterial
{
    float3 albedo;
    float roughness;
    float metalness;
};

cbuffer WorldMatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    PBRMaterial pbrMaterial;
    int PBRMode;
};

cbuffer SceneMatrixBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
    float4 cameraPos;
    int4 lightCount; // x - light count (max 10)
    float4 lightPos[10];
    float4 lightColor[10];
    float4 ambientColor;
};
