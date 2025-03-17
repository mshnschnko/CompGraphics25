static const float WhiteLumen = 11.2f;
static const float lumMin = 0;
static const float lumMax = 1000;

sampler SceneTextureSampler;
Texture2D SceneTexture;

struct PSInput
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 texcoord: TEXCOORD0;
};

cbuffer CBuf
{
	float4 averageLumen;
};

float3 Uncharted2Tonemap(float3 x)
{
	static const float A = 0.10;
	static const float B = 0.50;
	static const float C = 0.10;
	static const float D = 0.20;
	static const float E = 0.02;
	static const float F = 0.30;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float getExposition()
{
	float keyValue = 1.03f - 2.0f / (2.0f + log10(averageLumen.x + 1));
	return keyValue / (max(min(averageLumen.x, lumMax), lumMin));
}

float4 main(PSInput i) : SV_Target
{
	float3 color = SceneTexture.Sample(SceneTextureSampler, i.texcoord).rgb;

	float E = getExposition();
	float3 toneMappedCol = Uncharted2Tonemap(color * E * 8);
	float3 whiteScale = 1.0f / Uncharted2Tonemap(WhiteLumen);

	float4 final_color = float4(pow(abs(toneMappedCol * whiteScale), 1.0f / 2.2f), 1.0f);

	return final_color;
}
