#include "GlitterCommon.hlsli"

Texture2D gGBufferA	: register(t0);
Texture2D gGBufferB	: register(t1);
Texture2D gGBufferC	: register(t2);
Texture2D gGBufferD	: register(t3);
Texture2D<uint4> gGBufferE	: register(t4);
Texture2D<uint4> gGBufferF	: register(t5);

SamplerState samGBuffer : register(s0);

float2 OctNormalWrap(float2 v)
{
	return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}

float2 OctNormalEncode(float3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : OctNormalWrap(n.xy);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}

float3 OctNormalDecode(float2 encN)
{
	encN = encN * 2.0 - 1.0;

	float3 n;
	n.z = 1.0 - abs(encN.x) - abs(encN.y);
	n.xy = n.z >= 0.0 ? encN.xy : OctNormalWrap(encN.xy);
	n = normalize(n);
	return n;
}

struct GBufferData
{
	float3 worldNormal;
	float3 diffuseColor;
	float3 specularColor;
	float3 baseColor;
	float metallic;
	float specular;
	float roughness;
	float opacity;
	float depth;
	float cavity;
	float materialMask;
	float3 customT;
	float flakesFlagMipInterp;
	uint flakesFlagMipLower;
	uint flakesFlagMipHigher;
	uint2 flakesSeed;
};

GBufferData GBufferDecode(float4 gBufferA, float4 gBufferB, float4 gBufferC, float4 gBufferD, uint4 gBufferE, uint4 gBufferF)
{
	GBufferData data;
	data.worldNormal = OctNormalDecode(gBufferA.xy);

	data.baseColor = gBufferB.xyz;
	data.metallic = gBufferC.z;
	data.specular = gBufferC.x;
	data.opacity = gBufferB.w;

	data.roughness = gBufferA.w;
	data.depth = gBufferA.z;

	data.cavity = gBufferC.y;
	data.materialMask = gBufferC.w;

	data.diffuseColor = data.baseColor - data.baseColor * data.metallic;
	data.specularColor = lerp(0.08 * data.specular.xxx, data.baseColor, data.metallic);

	data.customT = OctNormalDecode(gBufferD.xy);
	data.flakesFlagMipInterp = gBufferD.z;
	data.flakesFlagMipLower = DecodeFlakesData(uint3(gBufferE.xy, 0));
	data.flakesFlagMipHigher = DecodeFlakesData(uint3(gBufferE.zw, 0));

	data.flakesSeed = gBufferF.xy;

	return data;
}

GBufferData GetGBufferData(float2 uv)
{
	float4 a = gGBufferA.SampleLevel(samGBuffer, uv, 0);
	float4 b = gGBufferB.SampleLevel(samGBuffer, uv, 0);
	float4 c = gGBufferC.SampleLevel(samGBuffer, uv, 0);
	float4 d = gGBufferD.SampleLevel(samGBuffer, uv, 0);

	//Exception for glitter
	uint gBufferMipMax;
	uint2 gBufferDims;
	gGBufferE.GetDimensions(0, gBufferDims.x, gBufferDims.y, gBufferMipMax);
	uint2 gBufferTex = gBufferDims * uv;
	uint4 e = gGBufferE.Load(uint3(gBufferTex, 0));

	gGBufferF.GetDimensions(0, gBufferDims.x, gBufferDims.y, gBufferMipMax);
	gBufferTex = gBufferDims * uv;
	uint4 f = gGBufferF.Load(uint3(gBufferTex, 0));

	return GBufferDecode(a, b, c, d, e, f);
}

float GetDepth(float2 uv)
{
	return gGBufferA.SampleLevel(samGBuffer, uv, 0).z;
}

float3 GetNormal(float2 uv)
{
	float4 a = gGBufferA.SampleLevel(samGBuffer, uv, 0);
	return OctNormalDecode(a.xy);
}