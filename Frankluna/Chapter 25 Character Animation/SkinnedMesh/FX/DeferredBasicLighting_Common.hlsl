#include "brdfhelper.hlsl"
#include "DeferredBasic_Common.hlsl"


// @param DiffSpecMask .r: diffuse, .g:specular e.g. float2(1,1) for both, float2(1,0) for diffuse only
float3 StandardShading(GBufferData GBuffer, float3 LobeRoughness, float3 LobeEnergy, float3 L, float3 V, half3 N, float2 DiffSpecMask)
{
	float3 H = normalize(V + L);
	float NoL = saturate( dot(N, L) );
	//float NoV = saturate( dot(N, V) );
	float NoV = abs( dot(N, V) ) + 1e-5;
	float NoH = saturate( dot(N, H) );
	float VoH = saturate( dot(V, H) );

	// Generalized microfacet specular
	float D = D_GGX( LobeRoughness[1], NoH ) * LobeEnergy[1];
	float Vis = Vis_SmithJointApprox( LobeRoughness[1], NoV, NoL );
	float3 F = F_Schlick( GBuffer.specularColor, VoH );

	return Diffuse_Lambert(GBuffer.diffuseColor) * (LobeEnergy[2] * DiffSpecMask.r) + (D * Vis * DiffSpecMask.g) * F;
}