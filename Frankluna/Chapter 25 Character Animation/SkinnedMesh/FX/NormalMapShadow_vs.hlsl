cbuffer cbPerFrame : register(b0)
{
	float3 gEyePosW			: packoffset(c0);
	float gHeightScale		: packoffset(c0.w);
	float gMaxTessDistance	: packoffset(c1.x);
	float gMinTessDistance	: packoffset(c1.y);
	float gMinTessFactor	: packoffset(c1.z);
	float gMaxTessFactor	: packoffset(c1.w);
};

cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld				: packoffset(c0);
	float4x4 gWorldInvTranspose	: packoffset(c4);
	float4x4 gViewProj			: packoffset(c8);
	float4x4 gWorldViewProj		: packoffset(c12);
	float4x4 gTexTransform		: packoffset(c16);
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float4 TangentL : TANGENT;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}