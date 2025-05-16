sampler TextureSampler;
Texture2D Texture;

static const float3 ChannelsWeight = float3(0.2126f, 0.7151f, 0.0722f);

struct PSInput
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 texcoord: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	float4 color = Texture.Sample(TextureSampler, i.texcoord);
	float brightness = log(dot(color.rgb, ChannelsWeight) + 1.0f);
	return float4(brightness, brightness, brightness, 1);
}
