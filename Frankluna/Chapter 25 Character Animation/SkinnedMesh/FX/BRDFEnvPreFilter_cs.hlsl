#include "brdfhelper.hlsl"

TextureCube gCubeMap	: register(t0);
SamplerState samLinear : register(s0);

cbuffer cbPerObject : register(b0)
{
	float gTargetSize : packoffset(c0);
	float gRoughness : packoffset(c0.y);
	float2 gPad : packoffset(c0.z);
};

cbuffer cbImmutable
{
	static float3 g_f3CubeMapPos[6] =
	{
		float3(1, 0, 0),
		float3(-1, 0, 0),
		float3(0, 1, 0),
		float3(0, -1, 0),
		float3(0, 0, 1),
		float3(0, 0, -1)
	};

	static float3 g_f3CubeMapUVPos[6][2] =
	{
		{ float3(0, 0, -1), float3(0, -1, 0) },
		{ float3(0, 0, 1), float3(0, -1, 0) },
		{ float3(1, 0, 0), float3(0, 0, 1) },
		{ float3(1, 0, 0), float3(0, 0, -1) },
		{ float3(1, 0, 0), float3(0, -1, 0) },
		{ float3(-1, 0, 0), float3(0, -1, 0) }
	};
};

float3 PrefilterEnvMap(float Roughness, float3 R)
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	float TotalWeight = 0.f;
	const uint NumSamples = 1024;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley2d(i, NumSamples);
			float3 H = ImportanceSampleGGX(Xi, Roughness, N);
			float3 L = 2 * dot(V, H) * H - V;
			float NoL = saturate(dot(N, L));
		if (NoL > 0)
		{
			float3 col = gCubeMap.SampleLevel(samLinear, L, 0).rgb;
			col = pow(col, 2.2f); // to linear-space
			PrefilteredColor += col * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

#define BLOCKSIZE_X 128
#define BLOCKSIZE_Y 1
#define BLOCKSIZE_Z 6

RWTexture2DArray<float4> result : register(u0);

[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_Z)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID
	, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint3 writeTarget;
	writeTarget.z = DTid.z;
	writeTarget.y = DTid.x / gTargetSize;
	writeTarget.x = DTid.x - writeTarget.y * gTargetSize;
	float3 toTarget = g_f3CubeMapPos[DTid.z] * gTargetSize * 0.5f
	+ g_f3CubeMapUVPos[DTid.z][0] * (writeTarget.x - gTargetSize * 0.5f)
	+ g_f3CubeMapUVPos[DTid.z][1] * (writeTarget.y - gTargetSize * 0.5f);
	result[writeTarget] = float4(PrefilterEnvMap(gRoughness, normalize(toTarget)), 1.0f);
}