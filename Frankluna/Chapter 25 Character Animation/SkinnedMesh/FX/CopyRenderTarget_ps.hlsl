Texture2D gSourceMap	: register(t0);

SamplerState samLinear : register(s0);

struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float3 col = gSourceMap.Sample(samLinear, pin.Tex).xyz;
	col = pow(saturate(col), 0.454545f); // gamma correction

	return float4(col, 1.0f);
}