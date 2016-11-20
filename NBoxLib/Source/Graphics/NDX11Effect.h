#pragma once
#include <DirectXMath.h>

#include "../Macro/Macro.h"
#include "../Template/NIDGenerator.h"
#include "NDX11.h"

#if (DIRECTX_MATH_VERSION < 305) && !defined(XM_CALLCONV)
#define XM_CALLCONV __fastcall
typedef const XMVECTOR& HXMVECTOR;
typedef const XMMATRIX& FXMMATRIX;
#endif

class NRenderCmdList;

class INDX11Effect
{
public:
	virtual ~INDX11Effect() {}

	virtual void VApplyToQueue(NRenderCmdList* list) = 0;
};

class INDX11EffectMatrices
{
public:
	virtual ~INDX11EffectMatrices() {}

	virtual void XM_CALLCONV VSetWorld(DirectX::FXMMATRIX value) = 0;
	virtual void XM_CALLCONV VSetView(DirectX::FXMMATRIX value) = 0;
	virtual void XM_CALLCONV VSetProjection(DirectX::FXMMATRIX value) = 0;
};

class INDX11EffectDirLights
{
public:
	virtual ~INDX11EffectDirLights() {}

	virtual void VSetLightingEnabled(BOOL value) = 0;
	virtual void VSetPerPixelLighting(BOOL value) = 0;
	virtual void XM_CALLCONV VSetAmbientLightColor(DirectX::FXMVECTOR value) = 0;

	virtual void VSetLightEnabled(UINT32 ind, BOOL value) = 0;
	virtual void XM_CALLCONV VSetLightDirection(UINT32 ind, DirectX::FXMVECTOR value) = 0;
	virtual void XM_CALLCONV VSetLightDiffuseColor(UINT32 ind, DirectX::FXMVECTOR value) = 0;
	virtual void XM_CALLCONV VSetLightSpecularColor(UINT32 ind, DirectX::FXMVECTOR value) = 0;

	virtual void VEnableDefaultLighting() = 0;

	static const int g_MaxDirectionalLights = 3;
};

class INDX11EffectFog
{
public:
	virtual ~INDX11EffectFog() {}
};

class INDX11EffectSkinnedMesh
{
public:
	virtual ~INDX11EffectSkinnedMesh() {}

	virtual void SetWeightsPerVertex(UINT32 value) = 0;
	virtual void XM_CALLCONV SetBoneTransforms(DirectX::XMMATRIX const* value, SIZE_T count) = 0;
	virtual void ResetBoneTransforms() = 0;

	static const int g_MaxBones = 72;
};