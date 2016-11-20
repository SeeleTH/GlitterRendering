#include "brdfhelper.hlsl"
#include "DeferredBasic_Common.hlsl"
#include "random.hlsl"

#include "GlitterCommon.hlsli"

SamplerState samLinear : register(s1);
SamplerState samDiscrete : register(s2);

Texture2D gAccumMap	: register(t4);
Texture2D gNormalMap	: register(t5);
Texture2D<uint4> gFlakesMap 	: register(t6);

cbuffer cbPerFrame : register(b0)
{
	float MaskMat : packoffset(c0);
	float3 padding : packoffset(c0.y);
}

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	//GBufferData data = GetGBufferData(pin.Tex);
	//float3 normalN = data.worldNormal;
	//float3 normalT = data.customT;
	//float3 normalB = normalize(cross(normalN, normalT));
	//float2 uv = data.customUV;
	//float matMask = data.materialMask;
	//float rand = ComputeRandomFrom2DPosition(uv*1000) * 0.5f + 0.5f;
	//float3 col = gAccumMap.Sample(samLinear, pin.Tex);
	//return float4(col * rand, 1.0f);
	float2 vTex = pin.Tex;
	//vTex.x = vTex.x * 2.f;
	uint3 sampleLoc;
	sampleLoc.x = vTex.x * 512.f;
	sampleLoc.y = vTex.y * 512.f;
	sampleLoc.z = 1;
	uint4 flakesRawData = gFlakesMap.Load(sampleLoc);
	//float4 flakesRawData = gFlakesMap.SampleLevel(samDiscrete, vTex, 0);
	//uint flakesFlagData = (uint)flakesRawData.y << 8 + (uint)flakesRawData.z;
	uint flakeFlag = DecodeFlakesData(flakesRawData.rgb);
	float3 flakeData = GetAvgVecFromFlakesFlag(flakeFlag);
	float3 normalRawData = gNormalMap.SampleLevel(samLinear, vTex, 0).rgb * 2.0f - 1.0f;
	//float3 testData = GetVecFromFlakesFlagBit(GetFlakesFlagBit(normalRawData));
	uint testData2 = GetFlakesFlagBit(pin.ScreenVector);
	//return float4(GetVecFromFlakesFlagBit(floor(pin.Tex.x * 4) + floor(pin.Tex.y *4)*4), 1.f);
	//return float4((GetVecFromFlakesFlagBit(testData2)+float3(1.f,1.f,1.f))*0.5f, 1.f);
	//return float4(testData2/16.f, 0.f, 0.f, 1.f);
	//return float4((atan2(pin.Tex.y*2.f-1.f, pin.Tex.x*2.f-1.f) + PI) / (2.f*PI), 0.f, 0.f, 1.f);
	//return floor(-pin.Tex.x + 1.5)*((float4(testData, 1.0f) + 1.f)*0.5f) + floor(pin.Tex.x + 0.5)*((float4(normalRawData, 1.0f) + 1.f)*0.5f);
	//return floor(-pin.Tex.x + 1.5)*((float4(testData, 1.0f) + 1.f)*0.5f) + floor(pin.Tex.x + 0.5)*((float4(flakeData, 1.0f) + 1.f)*0.5f);
	//return ((float4(normalRawData, 1.0f) + 1.f)*0.5f);
	return ((float4(flakeData, 1.0f) + 1.f)*0.5f);
}