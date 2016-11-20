cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld					: packoffset(c0);
	float4x4 gTexTransform			: packoffset(c4);
	float4x4 gWorldInvTranspose		: packoffset(c8);
	float4x4 gWorldView				: packoffset(c12);
	float4x4 gWorldViewProj			: packoffset(c16);
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
	float4 PosH       : SV_POSITION;
	float3 PosW       : POSITION0;
	float3 PosV       : POSITION1;
	float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	//float2 Depth	  : TEXCOORD1;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, gWorld);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	//vout.Depth.x = vout.PosH.z;
	//vout.Depth.y = vout.PosH.w;

	return vout;
}