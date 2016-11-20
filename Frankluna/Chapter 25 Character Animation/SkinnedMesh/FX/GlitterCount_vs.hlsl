
struct GLITTER_FLAKE
{
	float4 m_f4Pos;
	float4 m_f4Dir;
};

struct VertexOut
{
	//float4 PosH       : SV_POSITION;
	float3 Pos       : POSITION;
	float3 NormalW    : NORMAL;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld					: packoffset(c0);
	float4x4 gWorldInvTranspose		: packoffset(c4);
	float4x4 gWorldViewProj			: packoffset(c8);
};


StructuredBuffer<GLITTER_FLAKE>   g_bufGlitterFlake : register(t0);

VertexOut main(uint id : SV_VERTEXID)
{
	VertexOut vout;
	vout.Pos = g_bufGlitterFlake[id].m_f4Pos.xyz;
	vout.NormalW = mul(g_bufGlitterFlake[id].m_f4Dir.xyz, (float3x3)gWorldInvTranspose);
	//vout.PosH = mul(float4(g_bufGlitterFlake[id].m_f4Pos.xyz, 1.0f), gWorldViewProj);
	return vout;
}