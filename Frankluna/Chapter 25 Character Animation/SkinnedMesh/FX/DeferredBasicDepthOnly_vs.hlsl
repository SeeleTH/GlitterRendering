cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewProj	: packoffset(c0);
};

cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld				: packoffset(c0);
	float4x4 gTexTransform		: packoffset(c4);
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

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(mul(float4(vin.PosL, 1.0f), gWorld), gViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}