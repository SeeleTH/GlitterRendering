#include "LightHelper.fx"

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirLights[3]	: packoffset(c0);
	float3 gEyePosW					: packoffset(c12);

	float  gFogStart				: packoffset(c12.w);
	float  gFogRange				: packoffset(c13);
	float3 gPerFramePadding			: packoffset(c13.y);
	float4 gFogColor				: packoffset(c14);
};

cbuffer cbPerObject : register(b1)
{
	float4x4 gWorld					: packoffset(c0);
	float4x4 gWorldInvTranspose		: packoffset(c4);
	float4x4 gWorldViewProj			: packoffset(c8);
	float4x4 gWorldViewProjTex		: packoffset(c12);
	float4x4 gTexTransform			: packoffset(c16);
	float4x4 gShadowTransform		: packoffset(c20);
	Material gMaterial				: packoffset(c24);
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
	float3 PosW       : POSITION;
	float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
	float4 SsaoPosH   : TEXCOORD2;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	// Generate projective tex-coords to project shadow map onto scene.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	// Generate projective tex-coords to project SSAO map onto scene.
	vout.SsaoPosH = mul(float4(vin.PosL, 1.0f), gWorldViewProjTex);

	return vout;
}