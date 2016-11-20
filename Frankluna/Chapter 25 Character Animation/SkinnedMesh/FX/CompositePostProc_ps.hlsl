//#include "DeferredBasic_Common.hlsl"

SamplerState samLinear : register(s0);

Texture2D gAccumImage	: register(t0);
Texture2D gSSAOImage	: register(t1);


struct VertexOut
{
	float4 PosH       : SV_POSITION;
	float2 Tex        : TEXCOORD0;
	float3 ScreenVector : TEXCOORD1;
};
float4 main(VertexOut pin) : SV_TARGET
{
	//GBufferData data = GetGBufferData(pin.Tex);
	float4 col = gAccumImage.SampleLevel(samLinear, pin.Tex, 0.0);
	float ssao = gSSAOImage.Sample(samLinear, pin.Tex).r;
	return ssao * col;
}