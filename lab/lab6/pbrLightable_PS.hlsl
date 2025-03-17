#include "PBRBuffers.h"

// Model params
Texture2D roughnessTex : register (t0);
Texture2D normalTex : register (t1);
Texture2D FTex : register (t2);
SamplerState roughnessSmplr : register (s0);
SamplerState normalSmplr : register (s1);
SamplerState FTexSmplr : register (s2);

// Env params
TextureCube irrTex : register (t3);
TextureCube prefTex : register (t4);
Texture2D brdfTex : register (t5);
SamplerState envSmplr : register (s3);
SamplerState brdfSmplr : register (s4);

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

float normalDistribution(float3 wPos, float3 norm, int lightIdx, float roughness)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);

	float alpha = clamp(roughness, 0.001f, 1);
	float alphaSqr = sqr(alpha);

	float3 n = norm;//normalize(norm);
	return alphaSqr / (3.1415926 * sqr(sqr(posDot(n, h)) * (alphaSqr - 1) + 1));
}

float SchlickGGX(float3 n, float3 v, float k)
{
	float nv = posDot(n, v);
	return nv / (nv * (1 - k) + k);
}

float geometry(float3 wPos, float3 norm, int lightIdx, float roughness)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);
	float alpha = clamp(roughness, 0.001f, 1);
	float k = sqr(alpha + 1) / 8;

	float3 n = norm;//normalize(norm);
	return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}

float3 fresnel(float3 wPos, float3 norm, int lightIdx, float metalness, float dielectricF0, float3 albedo)
{
	float3 v = vecToCam(wPos);
	float3 l = vecToLight(lightPos[lightIdx].xyz, wPos);
	float3 h = normalize(l + v);

	float3 F0 = float3(dielectricF0, dielectricF0, dielectricF0) * (1 - metalness) +  albedo * metalness;
	return F0 + (1 - F0) * pow(1 - posDot(h, v), 5);
}

float3 FresnelSchlickRoughnessFunction(float3 wPos, float3 norm, float roughness, float metalness, float dielectricF0, float3 albedo)
{
	float3 F0 = lerp(float3(dielectricF0, dielectricF0, dielectricF0), albedo, metalness);
	float3 v = vecToCam(wPos);
	float3 ir = float3(1.f, 1.f, 1.f) * max(1 - roughness, 10e-3);
	return F0 + (max(ir, F0) - F0) * pow(1 - posDot(norm, v), 5);
}

float3 CountPBRColor(float3 wPos, float3 n, float3 v, float roughness, float metalness, float dielectricF0, float3 albedo) {
	float3 result = { 0.f, 0.f, 0.f };

	// Count lighning part
	for (uint i = 0; i < lightCount.x; ++i)
	{
		float3 l = vecToLight(lightPos[i].xyz, wPos);

		float D = normalDistribution(wPos, n, i, roughness);
		float G = geometry(wPos, n, i, roughness);
		float3 F = fresnel(wPos, n, i, metalness, dielectricF0, albedo);

		float3 result_add = { 0.f, 0.f, 0.f };
		result_add = (1 - F) * albedo / 3.1415926 * (1 - metalness) + D * F * G / (0.001f + 4 * (posDot(l, n) * posDot(v, n)));
		
		// dot(l, l) = ||l||^2 - как в законе обратных квадратов
		result += clamp(lightColor[i] * result_add * lightColor[i].w / (dot(l, l) + 0.01f) * (dot(l, n) > 0), 0.0f, 1.0f);
	}

	// Count IBL specular part
	float3 r = normalize(2.0f * dot(v, n) * n - v);
	static const float MAX_REFLECTION_LOD = 4.0;
	float3 prefilteredColor = prefTex.SampleLevel(envSmplr, r, roughness * MAX_REFLECTION_LOD);
	float3 F0 = lerp(float3(dielectricF0, dielectricF0, dielectricF0), albedo, metalness);
	float2 splArg = float2(max(dot(n, v), 0.0), roughness);
	float2 envBRDF = brdfTex.Sample(brdfSmplr, splArg);
	float3 specular = specular = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
	
	// Count IBL diffuse part
	float3 F = FresnelSchlickRoughnessFunction(wPos, n, roughness, metalness, dielectricF0, albedo);
	float3 kS = F;
	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metalness;
	float3 irradiance = irrTex.Sample(envSmplr, n).rgb;
	float3 diffuse = irradiance * albedo;
	float3 diffuseComponent = kD * diffuse;
	
	// Count total ambient light
	float3 ambient = diffuseComponent + specular;

	// return total color
	return result + ambient;
}

float4 main(PS_INPUT input) : SV_Target0{
	float3 n = normalize(input.normal.xyz);
	float3 v = vecToCam(input.worldPos);

	if (viewMode.y == 0) {
		float3 binorm = normalize(cross(input.normal, input.tangent));
		float3 localNorm = normalTex.Sample(normalSmplr, input.texUV).xyz * 2.0 - 1.0;
		n = localNorm.x * normalize(input.tangent) + localNorm.y * binorm + localNorm.z * normalize(input.normal);
	}

	float roughness = max(pbr.x, 0.001);
	float metalness = pbr.y;
	float2 mr = roughnessTex.Sample(roughnessSmplr, input.texUV).rg;
	if (viewMode.z == 0) {
		roughness = max(mr.g, 0.001);
		metalness = mr.r;
	}

	float dielectricF0 = pbr.z;
	float3 albd = albedo;
	float3 texColor = FTex.Sample(FTexSmplr, input.texUV);
	if (viewMode.w == 0) {
		albd = texColor;
	}

	float3 color = CountPBRColor(input.worldPos.xyz, n, v, roughness, metalness, dielectricF0, albd);

	if (viewMode.x == 1) {
		color = (n + 1) / 2;
	}
	if (viewMode.x == 2) {
		color = float3(mr.r, mr.g, 0);
	}
	if (viewMode.x == 3) {
		color = texColor;
	}

	return float4(color, 1);
}
