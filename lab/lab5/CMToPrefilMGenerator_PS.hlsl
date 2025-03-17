#include "IBLheader.h"

cbuffer PrefilConstantbuffer : register(b0)
{
	float4 roughness4;
};

TextureCube tex : register(t0);
SamplerState smplr : register(s0);

float sqr(float x)
{
	return x * x;
}

float posDot(float3 a, float3 b)
{
	return max(dot(a, b), 0);
}

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 norm, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * 3.1415926 * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	float3 H;
	H.x = cos(phi) * sinTheta;
	H.z = sin(phi) * sinTheta;
	H.y = cosTheta;

	float3 up = abs(norm.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, norm));
	float3 bitangent = cross(norm, tangent);
	float3 sampleVec = tangent * H.x + bitangent * H.z + norm * H.y;
	return normalize(sampleVec);
}

float normalDistribution(float3 halfV, float3 norm, float roughness)
{
	float3 n = normalize(norm);
	float alpha = min(max(roughness, 0.01f), 1);

	float alphaSqr = sqr(alpha);
	return alphaSqr / (3.1415926 * sqr(sqr(posDot(norm, halfV)) * (alphaSqr - 1) + 1));
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 norm = normalize(input.localPos);
	float3 view = norm;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0, 0, 0);

	float roughness = roughness4;
	static const uint SAMPLE_COUNT = 1024u;
	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, norm, roughness);
		float3 L = normalize(2.0 * dot(view, H) * H - view);
		float ndotl = max(dot(norm, L), 0.0);
		float ndoth = max(dot(norm, H), 0.0);
		float hdotv = max(dot(H, view), 0.0);

		float D = normalDistribution(H, norm, roughness);
		float pdf = (D * ndoth / (4.0 * hdotv)) + 0.0001;
		float resolution = 512.0;
		float saTexel = 4.0 * 3.1415926 / (6.0 * resolution * resolution);
		float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
		float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

		if (ndotl > 0.0) {
			prefilteredColor += tex.SampleLevel(smplr, L, mipLevel) * ndotl;
			totalWeight += ndotl;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	return float4(prefilteredColor, 1.0f);
}