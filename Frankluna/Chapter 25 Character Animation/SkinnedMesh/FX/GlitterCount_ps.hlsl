
struct GSOutput
{
	float4 PosH : SV_POSITION;
};

float4 main(GSOutput pin) : SV_TARGET
{
	return float4(0.3f,0.3f,0.3f,1.0f);
}