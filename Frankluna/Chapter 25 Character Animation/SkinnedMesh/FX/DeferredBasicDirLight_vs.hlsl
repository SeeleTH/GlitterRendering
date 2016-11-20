cbuffer cbPerFrame : register(b0)
{
	float4x4 gWorldViewProj : packoffset(c0);
	float4x4 gScreenToTranslatedWorld : packoffset(c4);
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float2 Tex      : TEXCOORD;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;
	vout.ScreenVector = mul(float4(vout.PosH.xy, 1.0f, 0.f), gScreenToTranslatedWorld).xyz;
	return vout;
}