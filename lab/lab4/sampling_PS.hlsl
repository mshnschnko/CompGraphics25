sampler TextureSampler;
Texture2D Texture;

struct PSInput
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 texcoord: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	float4 color = Texture.Sample(TextureSampler, i.texcoord);
	return color;
}
