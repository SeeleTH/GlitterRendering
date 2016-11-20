#include "GlitterCommon.hlsli"
#define MIPMAP_BOX 4
#define MIPMAP_BOX_SQ 2

Texture2D<uint4> source : register(t0);
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
	uint accumFlag = 0;
	if (writeTarget.x < gTargetSize.x && writeTarget.y < gTargetSize.y)
	{
		[unroll] for (uint u = 0; u < MIPMAP_BOX_SQ; u++)
		{
			[unroll] for (uint v = 0; v < MIPMAP_BOX_SQ; v++)
			{
				uint2 sourceTarget = writeTarget * 2 + uint2(u, v);
				uint4 sourceData = source.Load(uint3(sourceTarget, 0));
				uint flakeFlag = DecodeFlakesData(sourceData.rgb);
				accumFlag = accumFlag | flakeFlag;
			}
		}

		uint3 encodedFlag = EncodeFlakesData(accumFlag);
		result[writeTarget] = uint4(encodedFlag , 0);
		result2[writeTarget] = uint4(encodedFlag, 0);
	}
}