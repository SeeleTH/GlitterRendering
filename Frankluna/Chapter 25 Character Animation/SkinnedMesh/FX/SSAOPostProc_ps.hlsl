#include "brdfhelper.hlsl"
#include "DeferredBasic_Common.hlsl"
#include "random.hlsli"

#include "GlitterCommon.hlsli"

SamplerState samLinear : register(s1);
SamplerState samDiscrete : register(s2);

Texture2D gAccumMap	: register(t6);

cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewToTexSpace : packoffset(c0);
	float gFarZ : packoffset(c4);
	float3 padding : packoffset(c4.y);
}

static const float gOcclusionRadius = 0.5f;
static const float gOcclusionFadeStart = 0.2f;
static const float gOcclusionFadeEnd = 2.0f;
static const float gSurfaceEpsilon = 0.05f;
static const uint gSampleCount = 8;

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

float4 main(VertexOut pin) : SV_TARGET
{
	GBufferData data = GetGBufferData(pin.Tex);
	float3 normalN = data.worldNormal;
	float depth = data.depth;
	float3 p = depth * gFarZ * normalize(pin.ScreenVector);

	float3 UpVector = abs(normalN.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, normalN));
	float3 TangentY = cross(normalN, TangentX);
	float3x3 normalTBN = float3x3(TangentX, TangentY, normalN);

	float occlusionSum = 0.f;
	[unroll] for (uint i = 0; i < gSampleCount; i++)
	{
		float2 hamUV = hammersley2d(i, gSampleCount);
		float3 ham = hemisphereSample_uniform(hamUV.x, hamUV.y);
		float3 randVec = normalize(mul(ham, normalTBN));

		float3 q = p + randVec * gOcclusionRadius;
		float4 projQ = mul(float4(q,1.0f), gViewToTexSpace);
		projQ /= projQ.w;
		float rz = GetDepth(projQ.xy);
		float3 r = rz * gFarZ * normalize(q);
		//float distZ = p.z - r.z;
		float distZ = length(p - r);
		float dp = max(dot(normalN, normalize(r - p)), 0.f);
		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}
	occlusionSum /= gSampleCount;
	float access = 1.0f - occlusionSum;
	return float4(access ,0.f,0.f, 1.0f);
}