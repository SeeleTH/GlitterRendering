#ifndef RANDOM_HLSLI
#define RANDOM_HLSLI

// -------- ALU based method ---------

/*
* Pseudo random number generator, based on "TEA, a tiny Encrytion Algorithm"
* http://citeseer.ist.psu.edu/viewdoc/download?doi=10.1.1.45.281&rep=rep1&type=pdf
* @param v - old seed (full 32bit range)
* @param IterationCount - >=1, bigger numbers cost more performance but improve quality
* @return new seed
*/

uint2 ScrambleTEA(uint2 v, uint IterationCount = 3)
{
	// Start with some random data (numbers can be arbitrary but those have been used by others and seem to work well)
	uint k[4] = { 0xA341316Cu, 0xC8013EA4u, 0xAD90777Du, 0x7E95761Eu };

	uint y = v[0];
	uint z = v[1];
	uint sum = 0;

	[unroll] for (uint i = 0; i < IterationCount; ++i)
	{
		sum += 0x9e3779b9;
		y += (z << 4u) + k[0] ^ z + sum ^ (z >> 5u) + k[1];
		z += (y << 4u) + k[2] ^ y + sum ^ (y >> 5u) + k[3];
	}

	return uint2(y, z);
}

// Computes a pseudo random number for a given integer 2D position
// @param v - old seed (full 32bit range)
// @return random number in the range -1 .. 1
float ComputeRandomFrom2DPosition(uint2 v)
{
	return (ScrambleTEA(v).x & 0xffff) / (float)(0xffff) * 2 - 1;
}

// Computes a pseudo random number for a given integer 2D position
// @param v - old seed (full 32bit range)
// @return random number in the range -1 .. 1
float ComputeRandomFrom3DPosition(int3 v)
{
	// numbers found by experimentation
	return ComputeRandomFrom2DPosition(v.xy ^ (uint2(0x123456, 0x23446) * v.zx));
}

// Evaluate polynomial to get smooth transitions for Perlin noise
// only needed by Perlin functions in this file
// scalar(per component): 2 add, 5 mul
float4 PerlinRamp(float4 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

// bilinear filtered 2D noise, can be optimized
// @param v >0
// @return random number in the range -1 .. 1
float2 PerlinNoise2D_ALU(float2 fv)
{
	// floor is needed to avoid -1..1 getting the same value (int cast always rounds towards 0)
	int2 iv = int2(floor(fv));

		float2 a = ComputeRandomFrom2DPosition(iv + int2(0, 0));
		float2 b = ComputeRandomFrom2DPosition(iv + int2(1, 0));
		float2 c = ComputeRandomFrom2DPosition(iv + int2(0, 1));
		float2 d = ComputeRandomFrom2DPosition(iv + int2(1, 1));

		float2 Weights = PerlinRamp(float4(frac(fv), 0, 0)).xy;

		float2 e = lerp(a, b, Weights.x);
		float2 f = lerp(c, d, Weights.x);

		return lerp(e, f, Weights.y);
}

// bilinear filtered 2D noise, can be optimized
// @param v >0
// @return random number in the range -1 .. 1
float PerlinNoise3D_ALU(float3 fv)
{
	// floor is needed to avoid -1..1 getting the same value (int cast always rounds towards 0)
	int3 iv = int3(floor(fv));

		float2 a = ComputeRandomFrom3DPosition(iv + int3(0, 0, 0));
		float2 b = ComputeRandomFrom3DPosition(iv + int3(1, 0, 0));
		float2 c = ComputeRandomFrom3DPosition(iv + int3(0, 1, 0));
		float2 d = ComputeRandomFrom3DPosition(iv + int3(1, 1, 0));
		float2 e = ComputeRandomFrom3DPosition(iv + int3(0, 0, 1));
		float2 f = ComputeRandomFrom3DPosition(iv + int3(1, 0, 1));
		float2 g = ComputeRandomFrom3DPosition(iv + int3(0, 1, 1));
		float2 h = ComputeRandomFrom3DPosition(iv + int3(1, 1, 1));

		float3 Weights = PerlinRamp(frac(float4(fv, 0))).xyz;

		float2 i = lerp(lerp(a, b, Weights.x), lerp(c, d, Weights.x), Weights.y);
		float2 j = lerp(lerp(e, f, Weights.x), lerp(g, h, Weights.x), Weights.y);

		return lerp(i, j, Weights.z).x;
}

#endif