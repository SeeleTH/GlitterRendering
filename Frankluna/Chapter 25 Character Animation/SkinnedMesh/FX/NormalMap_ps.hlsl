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

cbuffer cbPerApp : register(b2)
{
	int gLightCount			: packoffset(c0);
	bool gUseTexure			: packoffset(c0.y);
	bool gAlphaClip			: packoffset(c0.z);
	bool gFogEnabled		: packoffset(c0.w);
	bool gReflectionEnabled : packoffset(c1);
	float3 gPerAppPadding	: packoffset(c1.y);
};

Texture2D gShadowMap	: register(t0);
Texture2D gDiffuseMap	: register(t1);
Texture2D gNormalMap	: register(t2);
Texture2D gSsaoMap		: register(t3);
TextureCube gCubeMap	: register(t4);

SamplerState samLinear				: register(s0);
SamplerComparisonState samShadow	: register(s1);

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

float4 PS(VertexOut pin) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);

		if (gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

	//
	// Lighting.
	//

	float4 litColor = texColor;
	if (gLightCount > 0)
	{
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Only the first light casts a shadow.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

		// Finish texture projection and sample SSAO map.
		pin.SsaoPosH /= pin.SsaoPosH.w;
		float ambientAccess = gSsaoMap.Sample(samLinear, pin.SsaoPosH.xy, 0.0f).r;

		// Sum the light contribution from each light source.  
		[unroll]
		for (int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye,
				A, D, S);

			ambient += ambientAccess*A;
			diffuse += shadow[i] * D;
			spec += shadow[i] * S;
		}

		litColor = texColor*(ambient + diffuse) + spec;

		if (gReflectionEnabled)
		{
			float3 incident = -toEye;
				float3 reflectionVector = reflect(incident, bumpedNormalW);
				float4 reflectionColor = gCubeMap.Sample(samLinear, reflectionVector);

				litColor += gMaterial.Reflect*reflectionColor;
		}
	}

	//
	// Fogging
	//

	if (gFogEnabled)
	{
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}