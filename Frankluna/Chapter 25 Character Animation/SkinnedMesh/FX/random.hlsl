#define SCRAMBLETEA_ITERATION 3

// @param xy should be a integer position (e.g. pixel position on the screen), repeats each 128x128 pixels
// similar to a texture lookup but is only ALU
float PseudoRandom(float2 xy)
{
	float2 pos = frac(xy / 128.0f) * 128.0f + float2(-64.340622f, -72.465622f);

		// found by experimentation
		return frac(dot(pos.xyx * pos.xyy, float3(20.390625f, 60.703125f, 2.4281209f)));
}

/*
* Pseudo random number generator, based on "TEA, a tiny Encrytion Algorithm"
* http://citeseer.ist.psu.edu/viewdoc/download?doi=10.1.1.45.281&rep=rep1&type=pdf
* @param v - old seed (full 32bit range)
* @param IterationCount - >=1, bigger numbers cost more performance but improve quality
* @return new seed
*/
uint2 ScrambleTEA(uint2 v)
{
	// Start with some random data (numbers can be arbitrary but those have been used by others and seem to work well)
	uint k[4] = { 0xA341316Cu, 0xC8013EA4u, 0xAD90777Du, 0x7E95761Eu };

	uint y = v[0];
	uint z = v[1];
	uint sum = 0;

	[UNROLL] for (uint i = 0; i < SCRAMBLETEA_ITERATION; ++i)
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