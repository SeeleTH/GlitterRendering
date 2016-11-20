#include "DeferredBasic_Common.hlsl"

SamplerState samInputImage : register(s1);

Texture2D gInputImage	: register(t6);

cbuffer cbPerFrame : register(b0)
{
	float2 gOffsetSize : packoffset(c0.x);
	float gFarZ : packoffset(c0.z);
	float padding : packoffset(c0.w);
}

cbuffer cvFixed
{
	static const float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
	static const int gBlurRadius = 5;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};
float4 main(VertexOut pin) : SV_TARGET
{
	GBufferData data = GetGBufferData(pin.Tex);
	float4 color = gWeights[5] * gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float totalWeight = gWeights[5];
	float3 centerNormal = data.worldNormal;
	float centerDepth = data.depth;

	[unroll] for (float i = -gBlurRadius; i <= gBlurRadius; i++)
	{
		if (abs(i) <= 0.0001f)
			continue;

		float2 tex = pin.Tex + i * gOffsetSize;
		float3 normal = GetNormal(tex);
		float depth = GetDepth(tex);
		if (dot(normal, centerNormal) >= 0.8f && abs(depth - centerDepth)*gFarZ <= 0.2f)
		{
			float weight = gWeights[i + 5];
			color += weight * gInputImage.SampleLevel(samInputImage, tex, 0.0);
			totalWeight += weight;
		}
	}

	return color / totalWeight;
}