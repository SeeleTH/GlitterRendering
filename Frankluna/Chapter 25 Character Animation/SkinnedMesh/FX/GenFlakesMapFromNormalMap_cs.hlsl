#include "GlitterCommon.hlsli"

Texture2D gNormalMap : register(t0);
SamplerState samLinear : register(s0);
RWTexture2D<uint4> result : register(u0);
RWTexture2D<uint4> result2 : register(u1);

cbuffer cbPerObject : register(b0)
{
	float2 gTargetSize : packoffset(c0);
	uint gMipLevel : packoffset(c0.z);
	float gPad : packoffset(c0.w);
};

cbuffer cbPerApp
{
	static const float gMinCosDistRoundToZero = 0.85f;
};

[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_Z)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID
	, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint2 writeTarget;
	writeTarget.y = DTid.x / gTargetSize.x;
	writeTarget.x = DTid.x - writeTarget.y * gTargetSize.x;
	float2 sampleLoc = float2(writeTarget.x / gTargetSize.x, writeTarget.y / gTargetSize.y);
	float3 normVec = gNormalMap.SampleLevel(samLinear, sampleLoc, 0).rgb*2.f - 1.f;
	normVec = float3(normVec.x, normVec.z, normVec.y);
	normVec = normalize(normVec);

	float3 upVec = float3(0.f, 1.f, 0.f);
	float cosDist = dot(normVec, upVec);
	uint4 rawResult = uint4(0, 0, 0, 0);
	if (abs(cosDist) < (1.0f - 0.25f*(1.f / (float)FLAKESBITS_YAXIS)))
	{
		uint normBit = GetFlakesFlagBit(normVec);
		//uint fullData = 1 << normBit;
		//uint firstHalf = fullData >> 8;
		//uint secondHalf = fullData & (0x00FF);
		rawResult = uint4(EncodeFlakesData(1 << normBit), 0);
	}
	result[writeTarget] = rawResult;
	result2[writeTarget] = rawResult;
	//result[writeTarget] = float4(asfloat(1 << normBit), (float)firstHalf, (float)secondHalf, 1.f);
}