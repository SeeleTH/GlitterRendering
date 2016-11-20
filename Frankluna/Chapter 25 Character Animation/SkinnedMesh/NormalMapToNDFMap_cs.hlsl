Texture2D gNormalMap	: register(t0);
SamplerState samLinear : register(s0);

#define BLOCKSIZE_X 1024
#define BLOCKSIZE_Y 1
#define BLOCKSIZE_Z 1

[numthreads(BLOCKSIZE_X, BLOCKSIZE_Y, BLOCKSIZE_Z)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID
	, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
}