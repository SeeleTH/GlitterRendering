#include "GlitterCommon.hlsli"
#include "random.hlsli"
#include "brdfhelper.hlsl"

SamplerState samLinear : register(s0);
RWTexture2D<uint4> result : register(u0);
RWTexture2D<uint4> result2 : register(u1);

cbuffer cbPerObject : register(b0)
{
	float2 gTargetSize : packoffset(c0);
	uint gMipLevel : packoffset(c0.z);
	float gPad : packoffset(c0.w);
};

[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_Z)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID
	, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{ 
	uint2 writeTarget;
	writeTarget.y = DTid.x / gTargetSize.x;
	writeTarget.x = DTid.x - writeTarget.y * gTargetSize.x;
	float2 sampleLoc = float2(writeTarget.x / gTargetSize.x, writeTarget.y / gTargetSize.y);
	float2 randValue;
	randValue.x = ComputeRandomFrom3DPosition(uint3(writeTarget.x, writeTarget.y, 1));
	randValue.y = ComputeRandomFrom3DPosition(uint3(writeTarget.x, writeTarget.y, 2));
	float3 normVec = hemisphereSample_cos(randValue.x, randValue.y);
	uint normBit = GetFlakesFlagBit(normVec);
	uint flakesBit = 1 << normBit;
	//uint fullData = 1 << normBit;
	//uint firstHalf = fullData >> 8;
	//uint secondHalf = fullData & (0x00FF);
	uint4 rawResult = uint4(EncodeFlakesData(flakesBit), 0);
		result[writeTarget] = rawResult;
	result2[writeTarget] = rawResult;
	//result[writeTarget] = float4(asfloat(1 << normBit), (float)firstHalf, (float)secondHalf, 1.f);
}