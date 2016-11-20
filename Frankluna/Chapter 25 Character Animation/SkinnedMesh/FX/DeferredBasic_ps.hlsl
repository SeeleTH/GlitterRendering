Texture2D gDiffuseMap	: register(t0);
Texture2D gNormalMap	: register(t1);
Texture2D gRoughnessSpecularMetallicCavityMap : register(t2);
Texture2D gMatMaskMap	: register(t3);
Texture2D<uint4> gFlakesMap	: register(t4);

SamplerState samLinear				: register(s0);

cbuffer cbPerFrame : register(b0)
{
	float gNearZ : packoffset(c0);
	float gFarZ : packoffset(c0.y);
}
cbuffer cbPerObject : register(b1)
{
	float3 gAlbedoColorMultiplier : packoffset(c0);
	float gRoughnessMultiplier : packoffset(c0.w);
	float gSpecularMultiplier : packoffset(c1);
	float gMetallicMultiplier : packoffset(c1.y);
	float gCavityMultiplier : packoffset(c1.z);
	float gMatMaskMultiplier : packoffset(c1.w);
	float gFlakesDensityMultiplier : packoffset(c2.x);
	float3 padding : packoffset(c2.y);
}

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float3 PosW       : POSITION0;
	float3 PosV       : POSITION1;
	float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	//float2 Depth	  : TEXCOORD1;
};


struct PSOutput
{
	float4 NormalDepthRoughness		: SV_Target0;
	float4 DiffuseAlbedo			: SV_Target1;
	float4 SpecularCavityMetallicMatmask 	: SV_Target2;
	float4 CustomVars 	: SV_Target3;
	uint4 CustomUint4 	: SV_Target4;
	uint4 CustomUint42 	: SV_Target5;
};

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

float GetMipLevel(float2 iUV, float2 iTextureSize)
{
	float2 dx = ddx(iUV * iTextureSize.x);
		float2 dy = ddy(iUV * iTextureSize.y);
		float d = max(dot(dx, dx), dot(dy, dy));
	return 0.5 * log2(d);
}

PSOutput main(VertexOut pin)
{
	PSOutput pout;

	pin.NormalW = normalize(pin.NormalW);
	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	normalMapSample = 2.0f * normalMapSample - 1.0f;
	float3 normalN = pin.NormalW;
		float3 normalT = normalize(pin.TangentW.xyz - dot(pin.TangentW.xyz, normalN)*normalN);
		float3 normalB = pin.TangentW.w*cross(normalN, normalT);
		float3x3 normalTBN = float3x3(normalT, normalB, normalN);
		float3 bumpedNormalW = mul(normalMapSample, normalTBN);

	float4 diffuseAlbedo = gDiffuseMap.Sample(samLinear, pin.Tex);
	diffuseAlbedo = pow(diffuseAlbedo, 2.2f); // to linear-space (should pre-process this but I'm too lazy)

	float4 matData = gRoughnessSpecularMetallicCavityMap.Sample(samLinear, pin.Tex);
	pout.NormalDepthRoughness.xy = OctNormalEncode(bumpedNormalW);
	pout.NormalDepthRoughness.z = length(pin.PosV) / gFarZ;//pin.Depth.x / pin.Depth.y;
	pout.NormalDepthRoughness.w = matData.r * gRoughnessMultiplier;
	pout.DiffuseAlbedo = diffuseAlbedo * float4(gAlbedoColorMultiplier, 1.0f);
	pout.SpecularCavityMetallicMatmask.x = matData.g * gSpecularMultiplier;
	pout.SpecularCavityMetallicMatmask.y = matData.a * gCavityMultiplier;
	pout.SpecularCavityMetallicMatmask.z = matData.b * gMetallicMultiplier;
	pout.SpecularCavityMetallicMatmask.w = gMatMaskMap.Sample(samLinear, pin.Tex).r * gMatMaskMultiplier;
	pout.CustomVars.xy = OctNormalEncode(normalT);
	pout.CustomVars.zw = pin.Tex * gFlakesDensityMultiplier;
	
	uint flakeMipMax;
	uint2 flakesMapDims;
	uint3 flakesTex;
	float2 flakeMapDimsMip;
	float flakeMip;
	uint3 flakesFlagRaw;
	gFlakesMap.GetDimensions(0, flakesMapDims.x, flakesMapDims.y, flakeMipMax);
	flakeMip = min(flakeMipMax-1,GetMipLevel(pin.Tex * gFlakesDensityMultiplier, flakesMapDims));
	flakesTex.z = floor(flakeMip);
	gFlakesMap.GetDimensions(flakesTex.z, flakeMapDimsMip.x, flakeMapDimsMip.y, flakeMipMax);
	flakesTex.x = gFlakesDensityMultiplier * pin.Tex.x * flakeMapDimsMip.x % flakeMapDimsMip.x;
	flakesTex.y = gFlakesDensityMultiplier * pin.Tex.y * flakeMapDimsMip.y % flakeMapDimsMip.y;
	flakesFlagRaw = (bool)gFlakesDensityMultiplier * gFlakesMap.Load(flakesTex).xyz;
	pout.CustomUint4.xy = flakesFlagRaw.xy;

	flakesTex.z = ceil(flakeMip);
	gFlakesMap.GetDimensions(flakesTex.z, flakeMapDimsMip.x, flakeMapDimsMip.y, flakeMipMax);
	flakesTex.x = gFlakesDensityMultiplier * pin.Tex.x * flakeMapDimsMip.x % flakeMapDimsMip.x;
	flakesTex.y = gFlakesDensityMultiplier * pin.Tex.y * flakeMapDimsMip.y % flakeMapDimsMip.y;
	flakesFlagRaw = (bool)gFlakesDensityMultiplier * gFlakesMap.Load(flakesTex).xyz;
	pout.CustomUint4.zw = flakesFlagRaw.xy;

	uint tempIntPart;
	pout.CustomVars.z = modf(flakeMip, tempIntPart);

	pout.CustomUint42 = uint4(sqrt(gFlakesDensityMultiplier) * pin.Tex.x * flakesMapDims.x % flakesMapDims.x
		, sqrt(gFlakesDensityMultiplier) * pin.Tex.y * flakesMapDims.y % flakesMapDims.y, 1, 1);

	return pout;
}