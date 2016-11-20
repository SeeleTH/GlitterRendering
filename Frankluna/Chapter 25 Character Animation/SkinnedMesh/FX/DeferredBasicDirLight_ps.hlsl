#include "DeferredBasicLighting_Common.hlsl"
#include "GlitterCommon.hlsli"
#include "random.hlsli"

//#define SHADE_ONLY_ONCE
//#define CHEATED_FLAKES
//#define CHEATED_FLAKES_2
//#define CHEATED_NO_SHADOW

Texture2D gShadowMap	: register(t6);
SamplerComparisonState samShadow	: register(s1);

cbuffer cbPerFrame : register(b0)
{
	float4x4 gLightShadowTransform : packoffset(c0);
	float4x4 gProjection : packoffset(c4);
	float3 gLightColor : packoffset(c8);
	float gNearZ : packoffset(c8.w);
	float3 gLightDir : packoffset(c9);
	float gFarZ : packoffset(c9.w);
	float3 gViewPos : packoffset(c10);
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	// Texel size.
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit /= 9.0f;
}

float4 main(VertexOut pin) : SV_TARGET
{
	GBufferData data = GetGBufferData(pin.Tex);
	float3 CameraVector = normalize(pin.ScreenVector);
	float WorldDepth = data.depth * gFarZ;
	float4 WorldPosition = float4(gViewPos + CameraVector * WorldDepth, 1.0f);
	float4 HShadowPos = mul(WorldPosition, gLightShadowTransform);
	float3 V = -CameraVector;
	float3 N = data.worldNormal;
	float3 ToLight = -gLightDir;
	float3 L = ToLight;	// no need to normalize
	float NoL = saturate(dot(N, L));

	float SurfaceAttenuation = 1.0f * CalcShadowFactor(samShadow, gShadowMap, HShadowPos);
	float3 LobeRoughness = float3(0, data.roughness, 1);
	float3 LobeEnergy = float3(1, 1, 1);
	float3 SurfaceLightingDiff = StandardShading(data, LobeRoughness, LobeEnergy, L, V, N, float2(1, 0));
	float3 SurfaceLightingSpec = StandardShading(data, LobeRoughness, LobeEnergy, L, V, N, float2(0, 1));
	float3 SurfaceLight = (SurfaceLightingDiff + SurfaceLightingSpec) * NoL * gLightColor * SurfaceAttenuation;
	// Glitter - begin
#ifdef CHEATED_NO_SHADOW
	SurfaceAttenuation = 1.f;
#endif
	float3 debugValue = float3(0,0,0);
	float3 GlitterLight = float3(0, 0, 0);
	if (data.flakesFlagMipLower | data.flakesFlagMipHigher != 0)
	{
		float3 T = data.customT;
		float3 B = normalize(cross(N, T));
		T = normalize(cross(N, B));
		float3x3 TBN = float3x3(T, B, N);
		float3 GLobeRoughness = float3(0, data.roughness, 1);
		float3 GLobeEnergy = float3(1, 1, 1);
		uint flakeCounter = 0;

		// rand test
		float2 offset;// = PerlinNoise2D_ALU(pin.Tex * 1000);
		//offset.x = ComputeRandomFrom3DPosition(uint3(pin.Tex.x * 1000, pin.Tex.y * 1000, pin.Tex.x * pin.Tex.y * 100));
		//offset.y = ComputeRandomFrom3DPosition(uint3(pin.Tex.x * 1000, pin.Tex.y * 1000, pin.Tex.x * pin.Tex.y * 200));

		offset.x = ComputeRandomFrom3DPosition(uint3(data.flakesSeed.x, data.flakesSeed.y, data.flakesSeed.x * data.flakesSeed.y));
		offset.y = ComputeRandomFrom3DPosition(uint3(data.flakesSeed.x, data.flakesSeed.y, data.flakesSeed.x * data.flakesSeed.y * 2));

#ifdef SHADE_ONLY_ONCE
		float avgNoL = 0.f;
		float3 maxFlakeLower = GetClosestDirFlake(data.flakesFlagMipLower, TBN, L, V, avgNoL, flakeCounter);
		float3 GlitterLightLower = StandardShading(data, GLobeRoughness, GLobeEnergy, L, V, maxFlakeLower, float2(1, 1))
		* gLightColor * SurfaceAttenuation * avgNoL * flakeCounter / (float)max(1, flakeCounter);

		float3 maxFlakeHigher = GetClosestDirFlake(data.flakesFlagMipHigher, TBN, L, V, avgNoL, flakeCounter);
		float3 GlitterLightHigher = StandardShading(data, GLobeRoughness, GLobeEnergy, L, V, maxFlakeHigher, float2(1, 1))
		* gLightColor * SurfaceAttenuation * avgNoL * flakeCounter / (float)max(1, flakeCounter);
#else
		float3 GlitterLightLower = float3(0, 0, 0);
		uint flakeData = data.flakesFlagMipLower;
#ifdef CHEATED_FLAKES
		flakeData = 1 << 30 | 1 << 29 | 1 << 28 | 1 << 27;
#endif
#ifdef CHEATED_FLAKES_2
		flakeData |= 1 << 30 | 1 << 29 | 1 << 28 | 1 << 27;
#endif
		for (uint i = 0; i < FLAKESBITS; i++)
		{
			uint check = (flakeData >> i) & 0x00000001;
			if (check)
			{
				float3 gN = normalize(GetVecFromFlakesFlagBit(i , offset));
					gN = float3(gN.x, gN.z, gN.y);
				gN = mul(gN, TBN);
				float3 GSurfaceLighting = StandardShading(data, GLobeRoughness, GLobeEnergy, L, V, gN, float2(1, 1));
					float GNoL = saturate(dot(gN, L));
				GlitterLightLower += check * GSurfaceLighting * GNoL;
				flakeCounter += check;
				debugValue += check * normalize(gN);
			}
		}
		debugValue = debugValue/(float)max(1, flakeCounter);
		GlitterLightLower = GlitterLightLower * gLightColor * SurfaceAttenuation / (float)max(1, flakeCounter);

		float3 GlitterLightHigher = float3(0, 0, 0);
		flakeData = data.flakesFlagMipHigher;
		flakeCounter = 0;
#ifdef CHEATED_FLAKES
		flakeData = 1 << 30 | 1 << 29 | 1 << 28 | 1 << 27;
#endif
#ifdef CHEATED_FLAKES_2
		flakeData |= 1 << 30 | 1 << 29 | 1 << 28 | 1 << 27;
#endif
		for (uint j = 0; j < FLAKESBITS; j++)
		{
			uint check = (flakeData >> j) & 0x00000001;
			if (check)
			{
				float3 gN = normalize(GetVecFromFlakesFlagBit(j , offset));
				gN = float3(gN.x, gN.z, gN.y);
				gN = mul(gN, TBN);
				float3 GSurfaceLighting = StandardShading(data, GLobeRoughness, GLobeEnergy, L, V, gN, float2(1, 1));
					float GNoL = saturate(dot(gN, L));
				GlitterLightHigher += check * GSurfaceLighting * GNoL;
				flakeCounter += check;
			}
		}
		GlitterLightHigher = GlitterLightHigher * gLightColor * SurfaceAttenuation / (float)max(1, flakeCounter);
#endif
		GlitterLight = lerp(GlitterLightLower, GlitterLightHigher, data.flakesFlagMipInterp);
	}
	else
	{
		GlitterLight = SurfaceLight;
	}
	// Glitter - end
	debugValue = abs(data.worldNormal);

	return float4(saturate(GlitterLight), 1.0f);
	//return float4(saturate(SurfaceLight), 1.0f);
	//return float4(saturate((pin.Tex.x < 0.5)*saturate(debugValue) + (pin.Tex.x > 0.5)*GlitterLight), 1.0f);
	//return float4(saturate((pin.Tex.x < 0.5)*SurfaceLight + (pin.Tex.x > 0.5)*GlitterLight), 1.0f);
}