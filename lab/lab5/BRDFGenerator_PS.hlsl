#include "IBLheader.h"

float sqr(float x)
{
  return x * x;
}
float posDot(float3 a, float3 b)
{
  return max(dot(a, b), 0);
}

float SchlickGGX(float3 n, float3 v, float k)
{
  float nv = posDot(n, v);
  return nv / (nv * (1 - k) + k);
}

float geometry(float3 n, float3 v, float3 l, float roughness)
{
  float alpha = min(max(roughness, 0.01f), 1);
  float k = sqr(alpha) / 2;

  return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
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


float2 IntegrateBRDF(float NdotV, float roughness)
{
  float3 V;
  V.x = sqrt(1.0 - NdotV);
  V.z = 0.0;
  V.y = NdotV;
  float A = 0.0;
  float B = 0.0;
  float3 N = float3(0.0, 1.0, 0.0);
  static const uint SAMPLE_COUNT = 1024u;

  for (uint i = 0u; i < SAMPLE_COUNT; ++i)
  {
    float2 Xi = Hammersley(i, SAMPLE_COUNT);
    float3 H = ImportanceSampleGGX(Xi, N, roughness);
    float3 L = normalize(2.0 * dot(V, H) * H - V);
    float NdotL = max(L.y, 0.0);
    float NdotH = max(H.y, 0.0);
    float VdotH = max(dot(V, H), 0.0);
    if (NdotL > 0.0)
    {
      float G = geometry(N, V, L, roughness);
      float G_Vis = (G * VdotH) / (NdotH * NdotV);
      float Fc = pow(1.0 - VdotH, 5.0);
      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  A /= float(SAMPLE_COUNT);
  B /= float(SAMPLE_COUNT);
  return float2(A, B);
}

float2 main(PS_INPUT input) : SV_TARGET
{
    float2 outpos = (float2(input.localPos.x, -input.localPos.y) + 1) / 2;
    float NdotV = outpos.x;
    float r = outpos.y;

    return IntegrateBRDF(NdotV, r);
}