struct VSOut
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 texcoord: TEXCOORD0;
};

VSOut main(float3 pos : Position, float3 color : Color, float2 texcoord : TEXCOORD0)
{
	VSOut vso;
	vso.pos = float4(pos, 1.0f);
	vso.color = float4(color, 1.0f);
	vso.texcoord = texcoord;
	return vso;
}
