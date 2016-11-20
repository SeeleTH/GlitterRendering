TextureCube gCubeMap: register(t0);

SamplerState samLinear : register(s0);

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float3 screenVec = normalize(pin.ScreenVector);
	float3 col = gCubeMap.Sample(samLinear, screenVec).xyz;
	col = pow(col, 2.2f); // to linear-space
	return float4(col, 1.0f);
}