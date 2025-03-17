#pragma once
#include <DirectXMath.h>

using namespace DirectX;

enum class PBRMode : int
{
	allPBR = 0,
	normal = 1,
	geom = 2,
	fresnel = 3,
};

struct PBRMaterial {
	XMFLOAT3 albedo;
	float roughness;
	float metalness;

	PBRMaterial(XMFLOAT3 albedo = XMFLOAT3(0.5f, 0.5f, 0.5f), float roughness = 0.5f, float metalness = 0.5f)
		: albedo(albedo), roughness(roughness), metalness(metalness) {};
};
