#ifndef GLITTER_COMMON_HLSLI
#define GLITTER_COMMON_HLSLI

#include "constants.hlsli"

#define BLOCKSIZE_X 1024
#define BLOCKSIZE_Y 1
#define BLOCKSIZE_Z 1
#define FLAKESBITS 32
#define FLAKESBITS_YAXIS 8
#define FLAKESBITS_XAXIS 4
#define UNZEROING 0.0001f

//#define USE_CONSTANTS

#ifdef USE_CONSTANTS
cbuffer cbFixed
{
	static const float3 gFlakeFlagVectors[FLAKESBITS] = 
	{
		float3(-0.37533f, 0.19509f, -0.906128f),
		float3(-0.906127f, 0.19509f, -0.37533f),
		float3(-0.906127f, 0.19509f, 0.37533f),
		float3(-0.37533f, 0.19509f, 0.906127f),
		float3(0.37533f, 0.19509f, 0.906127f),
		float3(0.906128f, 0.19509f, 0.37533f),
		float3(0.906127f, 0.19509f, -0.37533f),
		float3(0.37533f, 0.19509f, -0.906127f),
		float3(-0.31819f, 0.55557f, -0.768178f),
		float3(-0.768178f, 0.55557f, -0.31819f),
		float3(-0.768178f, 0.55557f, 0.31819f),
		float3(-0.31819f, 0.55557f, 0.768178f),
		float3(0.31819f, 0.55557f, 0.768178f),
		float3(0.768178f, 0.55557f, 0.318189f),
		float3(0.768178f, 0.55557f, -0.31819f),
		float3(0.31819f, 0.55557f, -0.768178f),
		float3(-0.212607f, 0.83147f, -0.51328f),
		float3(-0.51328f, 0.83147f, -0.212608f),
		float3(-0.51328f, 0.83147f, 0.212608f),
		float3(-0.212607f, 0.83147f, 0.51328f),
		float3(0.212607f, 0.83147f, 0.51328f),
		float3(0.51328f, 0.83147f, 0.212607f),
		float3(0.51328f, 0.83147f, -0.212608f),
		float3(0.212608f, 0.83147f, -0.51328f),
		float3(-0.0746578f, 0.980785f, -0.18024f),
		float3(-0.18024f, 0.980785f, -0.0746578f),
		float3(-0.18024f, 0.980785f, 0.0746578f),
		float3(-0.0746578f, 0.980785f, 0.18024f),
		float3(0.0746578f, 0.980785f, 0.18024f),
		float3(0.18024f, 0.980785f, 0.0746578f),
		float3(0.18024f, 0.980785f, -0.0746578f),
		float3(0.0746578f, 0.980785f, -0.18024f)
	};
};
#endif

uint GetFlakesFlagBit(float3 vec)
{
	vec = normalize(vec);
	float x_axis = asin(abs(vec.y));
	float y_axis = atan2(vec.z, vec.x) + PI;
	uint vert = floor((x_axis / (0.5f*PI)) * FLAKESBITS_XAXIS);
	uint hori = floor((y_axis / (2.f*PI)) * FLAKESBITS_YAXIS);
	uint flagBit = vert * FLAKESBITS_YAXIS + hori;
	return flagBit;
}

float3 GetVecFromFlakesFlagBit(uint bit)
{
#ifdef USE_CONSTANTS
	return gFlakeFlagVectors[bit];
#else
	uint vert = bit / FLAKESBITS_YAXIS;
	uint hori = bit - vert * FLAKESBITS_YAXIS;
	float x_axis_angle = ((float)vert + 0.5f) / (float)FLAKESBITS_XAXIS * 0.5f * PI;
	float y_axis_angle = ((float)hori + 0.5f) / (float)FLAKESBITS_YAXIS * 2.f * PI - PI;
	float3 result = float3(cos(x_axis_angle) * sin(y_axis_angle)
		, sin(x_axis_angle)
		, cos(x_axis_angle) * cos(y_axis_angle));
	return normalize(result);
#endif
}

float3 GetVecFromFlakesFlagBit(uint bit, float2 offset)
{
	uint vert = bit / FLAKESBITS_YAXIS;
	uint hori = bit - vert * FLAKESBITS_YAXIS;
	float x_axis_angle = ((float)vert + 0.5f + offset.x * 0.5f) / (float)FLAKESBITS_XAXIS * 0.5f * PI;
	float y_axis_angle = ((float)hori + 0.5f + offset.y * 0.5f) / (float)FLAKESBITS_YAXIS * 2.f * PI - PI;
	float3 result = float3(cos(x_axis_angle) * sin(y_axis_angle)
		, sin(x_axis_angle)
		, cos(x_axis_angle) * cos(y_axis_angle));
	return normalize(result);
}

float3 GetClosestDirFlake(uint flag, float3x3 TBN, float3 l, float3 v, out float avgNoL, out uint flakeCount)
{
	float3 result = float3(0.f,0.f,0.f);
	float accumNoL = 0.f;
	float H = normalize(l + v);
	float maxNoH = 0.f;
	uint count = 0;
	for (uint i = 0; i < FLAKESBITS; i++)
	{
		uint check = (flag >> i) & 0x00000001;
		float3 gN = normalize(GetVecFromFlakesFlagBit(i));
		gN = float3(gN.x, gN.z, gN.y);
		gN = mul(gN, TBN);
		float NoL = saturate(dot(gN, l));
		float NoH = saturate(dot(gN, H));
		accumNoL += check*NoL;
		count += check;
		uint isMax = check && (NoH > maxNoH);
		maxNoH = (isMax)* NoL + (!isMax)*maxNoH;
		result = (isMax)* gN + (!isMax)*result;
	}
	avgNoL = accumNoL / count;
	flakeCount = count;
	return result;
}

float3 GetAvgVecFromFlakesFlag(uint flag)
{
	float3 result = float3(0.f, 0.f, 0.f);
	[unroll] for (uint i = 0; i < FLAKESBITS; i++)
	{
		uint check = (flag >> i)&0x00000001;
		result += check * GetVecFromFlakesFlagBit(i);
	}
	return normalize(result);
}

uint3 EncodeFlakesData(uint flag)
{
	return uint3(flag >> 16, flag & 0x0000FFFF, 0);
}

uint DecodeFlakesData(uint3 rawData)
{
	return rawData.x << 16 | rawData.y;
}

#endif