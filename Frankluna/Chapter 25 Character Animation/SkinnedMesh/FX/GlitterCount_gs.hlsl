struct GLITTER_FLAKE
{
	float4 m_f4Pos;
	float4 m_f4Dir;
};

struct VertexOut
{
	float3 Pos       : POSITION;
	float3 NormalW    : NORMAL;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;
	//float3 NormalW    : NORMAL;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld					: packoffset(c0);
	float4x4 gWorldInvTranspose		: packoffset(c4);
	float4x4 gWorldViewProj			: packoffset(c8);
};

cbuffer cbPerFrame : register(b1)
{
	float4x4 gInvView				: packoffset(c0);
	float4 gLightDir				: packoffset(c4);
};

static const float PI = 3.14159265f;

cbuffer cbImmutable
{
	static float g_fConeCheckTheta = PI*5.f/180.f;
	static float g_fFlakeDefSize = 0.01f;
	static float3 g_f3FlakePos[4] =
	{
		float3(-1, 1, 0),
		float3(1, 1, 0),
		float3(-1, -1, 0),
		float3(1, -1, 0),
	};
};

StructuredBuffer<GLITTER_FLAKE>   g_bufGlitterFlake : register(t0);

[maxvertexcount(1)]
void main(
	point VertexOut input[1],
	inout PointStream< GSOutput > SpriteStream
)
{
	GSOutput element;
	float3 camPos = gInvView[3].xyz;
	float3 flakePos = input[0].Pos;
	float3 reflectDir = normalize(reflect(normalize(gLightDir.xyz), normalize(input[0].NormalW)));
	float3 viewDir = normalize(camPos - flakePos);
	float coneCheck = dot(reflectDir, viewDir);
	if (coneCheck > cos(g_fConeCheckTheta))
	{
		element.PosH = mul(float4(flakePos, 1.0f), gWorldViewProj);
		SpriteStream.Append(element);
	}
}

//[maxvertexcount(4)]
//void main(
//	point VertexOut input[1],
//	inout TriangleStream< GSOutput > SpriteStream
//	)
//{
//	GSOutput element;
//	float3 camPos = gInvView[3].xyz;
//	float3 flakePos = input[0].Pos;
//	float3 reflectDir = normalize(reflect(normalize(gLightDir.xyz), normalize(input[0].NormalW)));
//	float3 viewDir = normalize(camPos - flakePos);
//	float coneCheck = dot(reflectDir, viewDir);
//	if (coneCheck > cos(g_fConeCheckTheta))
//	{
//		//float size = distance(camPos, input[0].Pos);
//		for (uint i = 0; i < 4; i++)
//		{
//			float3 position = g_f3FlakePos[i] * 0.5f * g_fFlakeDefSize;
//			position = mul(position, (float3x3)gInvView) + flakePos;
//			element.PosH = mul(float4(position, 1.0f), gWorldViewProj);
//			SpriteStream.Append(element);
//		}
//		SpriteStream.RestartStrip();
//	}
//}