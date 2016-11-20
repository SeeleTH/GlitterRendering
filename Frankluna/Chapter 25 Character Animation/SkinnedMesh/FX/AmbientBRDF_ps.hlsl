#include "brdfhelper.hlsl"
#include "DeferredBasic_Common.hlsl"

#define GLITTER_SHADING

#ifdef GLITTER_SHADING
#include "GlitterCommon.hlsli"
#include "random.hlsli"
#endif

SamplerState samCubeMap : register(s1);

Texture2D gBrdfLutMap	: register(t6);
TextureCube gAmbientCubeMap	: register(t7);

cbuffer cbPerAmbientCubeMap : register(b0)
{
	float4 AmbientCubeMapColor : packoffset(c0);
	// x:mul, y:add, z:diffuseMip, w:mipCount
	float4 AmbientCubeMipAdjust : packoffset(c1);
}

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

half ComputeCubemapMipFromRoughness(half Roughness, half MipCount)
{
	// Level starting from 1x1 mip
	//half Level = 3 - 1.15 * log2(Roughness);
	//return MipCount - 1 - Level;
	return MipCount * Roughness;
}

half3 EnvBRDF(half3 SpecularColor, half Roughness, half NoV)
{
	float2 AB = gBrdfLutMap.SampleLevel(samCubeMap, float2(NoV, Roughness), 0).xy;
		float3 GF = SpecularColor * AB.x + saturate(50.0 * SpecularColor.y) * AB.y;
		return GF;
}

float4 main(VertexOut pin) : SV_TARGET
{
	GBufferData data = GetGBufferData(pin.Tex);
	float diffuseMip = AmbientCubeMipAdjust.z;
	float3 CameraVector = normalize(pin.ScreenVector);
	float3 V = -CameraVector;
	float3 N = data.worldNormal;
	float3 R = 2 * dot(V, N) * N - V;
	float NoV = abs(dot(N, V)) + 1e-5;

#ifdef GLITTER_SHADING
	float3 result = float3(0.f, 0.f, 0.f);
	if (data.flakesFlagMipLower | data.flakesFlagMipHigher != 0)
	{
		float3 T = data.customT;
		float3 B = normalize(cross(N, T));
		T = normalize(cross(N, B));
		float3x3 TBN = float3x3(T, B, N);
		float3 GLobeRoughness = float3(0, data.roughness, 1);
		float3 GLobeEnergy = float3(1, 1, 1);
		uint flakeCounter = 0;
		float2 offset;
		offset.x = ComputeRandomFrom3DPosition(uint3(data.flakesSeed.x, data.flakesSeed.y, data.flakesSeed.x * data.flakesSeed.y));
		offset.y = ComputeRandomFrom3DPosition(uint3(data.flakesSeed.x, data.flakesSeed.y, data.flakesSeed.x * data.flakesSeed.y * 2));
		float3 GlitterLightLower = float3(0, 0, 0);
		uint flakeData = data.flakesFlagMipLower;
		for (uint i = 0; i < FLAKESBITS; i++)
		{
			uint check = (flakeData >> i) & 0x00000001;
			if (check)
			{
				float3 gN = normalize(GetVecFromFlakesFlagBit(i, offset));
				gN = float3(gN.x, gN.z, gN.y);
				gN = mul(gN, TBN);

				float3 gR = 2 * dot(V, gN) * gN - V;
				float gNoV = abs(dot(gN, V)) + 1e-5;

				float3 diffuseContribution = 0;
				float3 specularContribution = 0;
				{
					diffuseContribution += data.diffuseColor * gAmbientCubeMap.SampleLevel(samCubeMap, gN, diffuseMip);
				}

				{
					float specMip = ComputeCubemapMipFromRoughness(data.roughness, AmbientCubeMipAdjust.w);
					float3 sampleSpecular = gAmbientCubeMap.SampleLevel(samCubeMap, gR, specMip).xyz;
					specularContribution += sampleSpecular * EnvBRDF(data.specularColor, data.roughness, gNoV);
				}
				GlitterLightLower += check * (diffuseContribution + specularContribution);
				flakeCounter += check;
			}
		}
		GlitterLightLower = GlitterLightLower / (float)max(1, flakeCounter);

		float3 GlitterLightHigher = float3(0, 0, 0);
		flakeData = data.flakesFlagMipHigher;
		flakeCounter = 0;
		for (uint i = 0; i < FLAKESBITS; i++)
		{
			uint check = (flakeData >> i) & 0x00000001;
			if (check)
			{
				float3 gN = normalize(GetVecFromFlakesFlagBit(i, offset));
				gN = float3(gN.x, gN.z, gN.y);
				gN = mul(gN, TBN);

				float3 gR = 2 * dot(V, gN) * gN - V;
				float gNoV = abs(dot(gN, V)) + 1e-5;

				float3 diffuseContribution = 0;
				float3 specularContribution = 0;
				{
					diffuseContribution += data.diffuseColor * gAmbientCubeMap.SampleLevel(samCubeMap, gN, diffuseMip);
				}

				{
					float specMip = ComputeCubemapMipFromRoughness(data.roughness, AmbientCubeMipAdjust.w);
					float3 sampleSpecular = gAmbientCubeMap.SampleLevel(samCubeMap, gR, specMip).xyz;
						specularContribution += sampleSpecular * EnvBRDF(data.specularColor, data.roughness, gNoV);
				}
				GlitterLightHigher += check * (diffuseContribution + specularContribution);
				flakeCounter += check;
			}
		}
		GlitterLightHigher = GlitterLightHigher / (float)max(1, flakeCounter);
		result = lerp(GlitterLightLower, GlitterLightHigher, data.flakesFlagMipInterp);
	}
	else
	{
		float3 diffuseContribution = 0;
		float3 specularContribution = 0;
		{
			diffuseContribution += data.diffuseColor * gAmbientCubeMap.SampleLevel(samCubeMap, N, diffuseMip);
		}

		{
			float specMip = ComputeCubemapMipFromRoughness(data.roughness, AmbientCubeMipAdjust.w);
			float3 sampleSpecular = gAmbientCubeMap.SampleLevel(samCubeMap, R, specMip).xyz;
				specularContribution += sampleSpecular * EnvBRDF(data.specularColor, data.roughness, NoV);
		}

		result = diffuseContribution + specularContribution;
	}
	result *= AmbientCubeMapColor;
	return saturate(float4(result, 1.0f));
#else
	float3 diffuseContribution = 0;
	float3 specularContribution = 0;
	{
		diffuseContribution += data.diffuseColor * gAmbientCubeMap.SampleLevel(samCubeMap, N, diffuseMip);
	}

	{
		float specMip = ComputeCubemapMipFromRoughness(data.roughness, AmbientCubeMipAdjust.w);
		float3 sampleSpecular = gAmbientCubeMap.SampleLevel(samCubeMap, R, specMip).xyz;
		specularContribution += sampleSpecular * EnvBRDF(data.specularColor, data.roughness, NoV);
	}

	float4 result = float4(diffuseContribution, 1.0f) + float4(specularContribution, 1.0f);
	result *= AmbientCubeMapColor;
	return saturate(result);
#endif
}