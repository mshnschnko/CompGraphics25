#include "PBRBuffers.hlsli"

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float3 normal : NORMAL;
};

float sqr(float x)
{
  return x * x;
}

float3 vecToCam(float3 wPos)
{
	float3 camPos = cameraPos.xyz;
	return normalize(camPos - wPos);
}

float3 vecToLight(float3 lightPos, float3 wPos)
{
	return normalize(lightPos - wPos);
}
float posDot(float3 a, float3 b)
{
	return max(dot(a, b), 0);
}

float normalDistribution(float3 wPos, float3 norm, int lightIdx)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);
	float alpha = clamp(pbrMaterial.roughness, 0.001f, 1);
	float alphaSqr = sqr(alpha);

	float3 n = norm;//normalize(norm);
	return alphaSqr / (3.1415926 * sqr(sqr(posDot(n, h)) * (alphaSqr - 1) + 1));
}

float SchlickGGX(float3 n, float3 v, float k)
{
	float nv = posDot(n, v);
	return nv / (nv * (1 - k) + k);
}

float geometry(float3 wPos, float3 norm, int lightIdx)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);
	float alpha = clamp(pbrMaterial.roughness, 0.001f, 1);
	float k = sqr(alpha + 1) / 8;

	float3 n = norm;//normalize(norm);
	return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}

float3 fresnel(float3 wPos, float3 norm, int lightIdx)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);

	float3 F0 = float3(0.04f, 0.04f, 0.04f) * (1 - pbrMaterial.metalness) +  pbrMaterial.albedo * pbrMaterial.metalness;
	return F0 + (1 - F0) * pow(1 - posDot(h, v), 5);
}

float4 main(PS_INPUT input) : SV_Target0{
  float3 result = { 0.f, 0.f, 0.f };
	
	for (uint i = 0; i < lightCount.x; ++i)
	{
		float3 v = vecToCam(input.worldPos);
		float3 l = vecToLight(lightPos[i].xyz, input.worldPos);
		float3 n = normalize(input.normal.xyz);

		float D = normalDistribution(input.worldPos.xyz, n, i);
		float G = geometry(input.worldPos.xyz, n, i);
		float3 F = fresnel(input.worldPos.xyz, n, i);

		float3 result_add = {0.f, 0.f, 0.f};
		if (PBRMode == 0)      // all
			result_add = (1 - F) * pbrMaterial.albedo / 3.1415926 * (1 - pbrMaterial.metalness) + D * F * G / (0.001f + 4 * (posDot(l, n) * posDot(v, n)));
		else if (PBRMode == 1) // norm distribution
			result_add = D;
		else if (PBRMode == 2) // geom
			result_add = G;
		else                   // fresnel
			result_add = F;

		// dot(l, l) = ||l||^2 - как в законе обратных квадратов
		result += lightColor[i] * result_add * lightColor[i].w / (dot(l, l) + 0.01f) * clamp(dot(l, n), 0.0f, 1.0f);
	}

  return float4(result, 1.0);
}