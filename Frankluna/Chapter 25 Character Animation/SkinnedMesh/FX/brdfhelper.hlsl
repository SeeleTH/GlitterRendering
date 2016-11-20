#include "constants.hlsli"

// ==================
// Hammersley - Begin
// ==================

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 hammersley2d(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

float3 hemisphereSample_uniform(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = 1.0 - u;
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float3 hemisphereSample_cos(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// ================
// Hammersley - End
// ================


// =================
// EPIC BRDF - Begin
// =================


float3 Diffuse_Lambert(float3 DiffuseColor)
{
	return DiffuseColor * (1 / PI);
}

float Vis_SmithJointApprox(float Roughness, float NoV, float NoL)
{
	float a = Roughness * Roughness;
	float Vis_SmithV = NoL * (NoV * (1 - a) + a);
	float Vis_SmithL = NoV * (NoL * (1 - a) + a);
	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
	//float k = (Roughness * Roughness) / 2.0f; // (Roughness + 1) * (Roughness + 1) / 8.f;
	//return (NoV / (NoV * (1 - k) + k))*(NoL / (NoL * (1 - k) + k));
}

float D_GGX(float Roughness, float NoH)
{
	float m = Roughness * Roughness;
	float m2 = m*m;
	float d = (NoH * m2 - NoH) * NoH + 1;
	return m2 / (PI*d*d);
}

float3 F_Schlick(float3 SpecularColor, float VoH)
{
	float Fc = pow(1 - VoH, 5);
	return saturate( 50.0 * SpecularColor.g) * Fc + (1 - Fc) * SpecularColor;
}

float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 TangentX = normalize(cross(UpVector, N));
		float3 TangentY = cross(N, TangentX);
		// Tangent to world space
		return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 Diffuse(float3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH)
{
	return Diffuse_Lambert(DiffuseColor);
}

float Distribution(float Roughness, float NoH)
{
	return D_GGX(Roughness, NoH);
}

float GeometricVisibility(float Roughness, float NoV, float NoL, float VoH)
{
	return Vis_SmithJointApprox(Roughness, NoV, NoL);
}

float3 Fresnel(float3 SpecularColor, float VoH)
{
	return F_Schlick(SpecularColor, VoH);
}

//float3 PrefilterEnvMap(float Roughness, float3 R)
//{
//	float3 N = R;
//		float3 V = R;
//		float3 PrefilteredColor = 0;
//		const uint NumSamples = 1024;
//	for (uint i = 0; i < NumSamples; i++)
//	{
//		float2 Xi = hammersley2d(i, NumSamples);
//			float3 H = ImportanceSampleGGX(Xi, Roughness, N);
//			float3 L = 2 * dot(V, H) * H - V;
//			float NoL = saturate(dot(N, L));
//		if (NoL > 0)
//		{
//			PrefilteredColor += EnvMap.SampleLevel(EnvMapSampler, L, 0).rgb * NoL;
//			TotalWeight += NoL;
//		}
//	}
//	return PrefilteredColor / TotalWeight;
//}

float2 IntegrateBRDF(float Roughness, float NoV)
{
	float3 V;
	V.x = sqrt(1.0f - NoV * NoV); // sin
	V.y = 0;
	V.z = NoV; // cos
	float A = 0;
	float B = 0;
	const uint NumSamples = 1024;
	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley2d(i, NumSamples);
			float3 H = ImportanceSampleGGX(Xi, Roughness, float3(0.f, 0.f, 1.f));
			float3 L = 2 * dot(V, H) * H - V;
			float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));
		if (NoL > 0)
		{
			float G = Vis_SmithJointApprox(Roughness, NoV, NoL);
			float G_Vis = NoL * G * (4 * VoH / NoH);//G * VoH / (NoH * NoV);
			float Fc = pow(1 - VoH, 5);
			A += (1 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	return float2(A, B) / NumSamples;
}

//float3 ApproximateSpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
//{
//	float NoV = saturate(dot(N, V));
//	float3 R = 2 * dot(V, N) * N - V;
//	float3 PrefilteredColor = PrefilterEnvMap(Roughness, R);
//	float2 EnvBRDF = IntegrateBRDF(Roughness, NoV);
//	return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
//}

// =================
// EPIC BRDF - End
// =================