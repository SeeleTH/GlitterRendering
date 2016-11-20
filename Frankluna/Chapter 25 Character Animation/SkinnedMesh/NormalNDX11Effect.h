#pragma once
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include <memory>
#include <DirectXMath.h>

class NResCache;
class NResHandle;

class NormalNDX11Effect : public INDX11Effect, INDX11EffectMatrices
{
public:
	NormalNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap);
	~NormalNDX11Effect();

	void VApplyToQueue(NRenderCmdList* list) override;

	void XM_CALLCONV VSetWorld(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetView(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetProjection(DirectX::FXMMATRIX value) override;

protected:

	struct DirectionalLight
	{
		DirectionalLight() { ZeroMemory(this, sizeof(this)); }

		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular;
		DirectX::XMFLOAT3 Direction;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};

	struct Material
	{
		Material() { ZeroMemory(this, sizeof(this)); }

		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular; // w = SpecPower
		DirectX::XMFLOAT4 Reflect;
	};

	struct PosNormalTexTan
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
		DirectX::XMFLOAT4 TangentU;
	};

	struct CPPERFRAME
	{
		DirectionalLight m_dirLight[3];
		DirectX::XMFLOAT3 m_f3EyePosW;
		float m_fFogStart;
		float m_fForRange;
		DirectX::XMFLOAT3 m_f3PerFramePadding;
		DirectX::XMFLOAT4 m_f4FogColor;
	};

	struct CPPEROBJECT
	{
		CPPEROBJECT()
		{
			DirectX::XMStoreFloat4x4(&m_matWorld, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matInvTranspose, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matWorldViewProj, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matWorldViewProjTex, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matTexTransform, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matShadowTransform, DirectX::XMMatrixIdentity());
		}

		DirectX::XMFLOAT4X4 m_matWorld;
		DirectX::XMFLOAT4X4 m_matInvTranspose;
		DirectX::XMFLOAT4X4 m_matWorldViewProj;
		DirectX::XMFLOAT4X4 m_matWorldViewProjTex;
		DirectX::XMFLOAT4X4 m_matTexTransform;
		DirectX::XMFLOAT4X4 m_matShadowTransform;
		Material m_material;
	};

	struct CPPERAPP
	{
		INT m_iLightCount;
		BOOL m_bUseTexture;
		BOOL m_bAlphaClip;
		BOOL m_bFogEnabled;
		BOOL m_bReflectionEnabled;
		DirectX::XMFLOAT3 m_f3PerAppPadding;
	};

	struct BOUNDINGSPHERE
	{
		BOUNDINGSPHERE() : m_vCenter(0.0f, 0.0f, 0.0f), m_fRadius(0.0f) {}
		DirectX::XMFLOAT3 m_vCenter;
		float m_fRadius;
	};

	struct SHADOW_PERFRAME
	{
		DirectX::XMFLOAT3 m_d3EyePosW;
		float m_fHeightScale;
		float m_fMaxTressDistance;
		float m_fMinTressDistance;
		float m_fMinTressFactor;
		float m_fMaxTressFactor;
	};

	struct SHADOW_PEROBJECT
	{
		DirectX::XMFLOAT4X4 m_matWorld;
		DirectX::XMFLOAT4X4 m_matInvTranspose;
		DirectX::XMFLOAT4X4 m_matViewProj;
		DirectX::XMFLOAT4X4 m_matWorldViewProj;
		DirectX::XMFLOAT4X4 m_matTexTransform;
	};

	struct RENDERITEM
	{
		DirectX::XMFLOAT4X4 m_matWorld;
		UINT32 m_uIndexOffset;
		UINT32 m_uIndexCount;
		UINT32 m_uVertexOffset;
		UINT32 m_uDiffuseMapIndex;
		UINT32 m_uNormalMapIndex;
		UINT32 m_uCubeMapIndex;
		CPPEROBJECT m_buffer;
		SHADOW_PEROBJECT m_shadowBuffer;
	};

	void BuildBuffers(NGraphicsDevice* dev);
	void UpdateBuffers();
	void UpdateBuffer(RENDERITEM& buffer);
	void BuildShadowTransform();

	DirectX::XMFLOAT4X4 m_matWorld;
	DirectX::XMFLOAT4X4 m_matView;
	DirectX::XMFLOAT4X4 m_matProjection;
	DirectX::XMFLOAT4X4 m_matLightView;
	DirectX::XMFLOAT4X4 m_matLightProj;
	DirectX::XMFLOAT4X4 m_matShadowTransform;

	NVertexBuffer m_vertBuff;
	NIndexBuffer m_indBuff;

	NConstantBuffer m_cBufferPerFrame;
	NConstantBuffer m_cBufferPerObject;
	NConstantBuffer m_cBufferPerApp;
	CPPERAPP m_sBufferPerApp;
	CPPERFRAME m_sBufferPerFrame;


	NConstantBuffer m_cShadowBufferPerFrame;
	NConstantBuffer m_cShadowBufferPerObject;
	SHADOW_PERFRAME m_sShadowBufferPerFrame;

	NTexture2DRes* m_pShadowMap;
	NTexture2DRes* m_pSSAOMap;

	BOUNDINGSPHERE m_sSceneBound;
	N_G_VIEWPORT m_sShadowViewport;

	std::vector<RENDERITEM> m_sRenderItems;
	std::vector<std::shared_ptr<NResHandle>> m_pTextures;

	std::shared_ptr<NResHandle> m_pVS;
	std::shared_ptr<NResHandle> m_pPS;
	std::shared_ptr<NResHandle> m_pShadowVS;
	std::shared_ptr<NResHandle> m_pShadowPS;

	UINT32 m_u32BoxVertexOffset;
	UINT32 m_u32GridVertexOffset;
	UINT32 m_u32SphereVertexOffset;
	UINT32 m_u32CylinderVertexOffset;

	UINT32 m_u32BoxIndexCount;
	UINT32 m_u32GridIndexCount;
	UINT32 m_u32SphereIndexCount;
	UINT32 m_u32CylinderIndexCount;

	UINT32 m_u32BoxIndexOffset;
	UINT32 m_u32GridIndexOffset;
	UINT32 m_u32SphereIndexOffset;
	UINT32 m_u32CylinderIndexOffset;

	// ================
	// Glitter - Begin
	// ================

	struct GLITTER_FLAKE
	{
		DirectX::XMFLOAT4 m_f4Pos;
		DirectX::XMFLOAT4 m_f4Dir;
	};

	struct CP_GLITTER_PERFRAME
	{
		DirectX::XMFLOAT4X4 m_matInvView;
		DirectX::XMFLOAT4 m_f4LightDir;
	};

	struct CP_GLITTER_PEROBJ
	{
		DirectX::XMFLOAT4X4 m_matWorld;
		DirectX::XMFLOAT4X4 m_matInvTranspose;
		DirectX::XMFLOAT4X4 m_matWorldViewProj;
	};

	std::shared_ptr<NResHandle> m_pGlitterComputeCS;
	std::shared_ptr<NResHandle> m_pGlitterCountVS;
	std::shared_ptr<NResHandle> m_pGlitterCountGS;
	std::shared_ptr<NResHandle> m_pGlitterCountPS;

	static UINT32 g_u32FlakesNumber;
	NStructureBuffer m_pFlakesDataMap;
	NTexture2DRes* m_pFlakesCountMap;

	CP_GLITTER_PERFRAME m_glitterPerFrame;
	CP_GLITTER_PEROBJ m_glitterPerObject;
	NConstantBuffer m_cGlitterBufferPerFrame;
	NConstantBuffer m_cGlitterBufferPerObject;

	void BuildGlitterBuffer(NGraphicsDevice* dev);

	// ================
	// Glitter - End
	// ================
};