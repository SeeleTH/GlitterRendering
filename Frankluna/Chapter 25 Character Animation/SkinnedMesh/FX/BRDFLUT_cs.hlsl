#include "brdfhelper.hlsl"

#define BLOCKSIZE_X 1024
#define BLOCKSIZE_Y 1
#define RESULTSIZE 1024

RWTexture2D<float2> brdfLutResult : register(u0);

[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint2 pixel = uint2(DTid.x, DTid.y);
	float2 calcParameter = float2(float(pixel.x / float(RESULTSIZE)), float(pixel.y / float(RESULTSIZE)));
	brdfLutResult[pixel] = IntegrateBRDF(calcParameter.x, calcParameter.y);
}

//RWTexture2D<float4> brdfLutResult : register(u0);
//
//[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, 1)]
//void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
//{
//	uint2 pixel = uint2(DTid.x, DTid.y);
//	float2 calcParameter = float2(float(pixel.x / float(RESULTSIZE)), float(pixel.y / float(RESULTSIZE)));
//	float2 result = IntegrateBRDF(calcParameter.x, calcParameter.y);
//	brdfLutResult[pixel] = float4(result.x, result.y, 0.0f, 1.0f);
//}