//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Effect used to shade sky dome.
//=============================================================================

// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap: register(t0);
SamplerState samLinear : register(s0);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : TEXCOORD0;
};

float4 PS(VertexOut pin) : SV_Target
{
	return gCubeMap.Sample(samLinear, pin.PosL);
}