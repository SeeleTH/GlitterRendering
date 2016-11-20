#pragma once


//#define GEN_PREFILTER_ENVMAP
//#define GEN_FLAKES_MAP

#ifdef GEN_FLAKES_MAP
//#define GEN_FLAKES_MAP_RANDOM
#ifndef GEN_FLAKES_MAP_RANDOM
#define GEN_FLAKES_MAP_NORMAL "Textures\\Normals\\brushedmetal.png"
#else
#define GEN_FLAKES_MAP_RANDOM_N 1
#endif
#define GEN_FLAKES_MAP_OUTPUT "Textures\\Flakes\\brushedmetalRounded.dds"
#endif

#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include <memory>
#include <DirectXMath.h>

class NResCache;
class NResHandle;
class BrdfLutMapNEffect;
class NGlitterModelProxy;

#ifdef GEN_PREFILTER_ENVMAP
class PrefilterEnvMapNEffect;
#endif
#ifdef GEN_FLAKES_MAP
class GenFlakesMapNDX11Effect;
#endif

class PBRNDX11Effect : public INDX11Effect, INDX11EffectMatrices
{
public:
	PBRNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap);
	~PBRNDX11Effect();

	void VApplyToQueue(NRenderCmdList* list) override;

	void XM_CALLCONV VSetWorld(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetView(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetProjection(DirectX::FXMMATRIX value) override;
	inline void SetCamNearZFarZ(float nz, float fz) { m_sPSBufferPerFrame.m_fNearZ = nz; m_sPSBufferPerFrame.m_fFarZ = fz; }
	inline void SetCamPos(float x, float y, float z){ m_f3CamPos.x = x; m_f3CamPos.y = y; m_f3CamPos.z = z; }

	inline void SwitchAccumBufferInd()
	{
		m_u32CurrentLightAccumInd = !m_u32CurrentLightAccumInd;
	}
	inline NTexture2DRes* GetCurrentAccumBuffer()
	{
		return m_pLightAccumBuffers[m_u32CurrentLightAccumInd];
	}

protected:

	struct PosNormalTexTan
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
		DirectX::XMFLOAT4 TangentU;
	};

	struct PosTex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT2 Tex;
	};

	struct CBPEROBJECT_VS
	{
		CBPEROBJECT_VS()
		{
			DirectX::XMStoreFloat4x4(&m_matWorld, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matInvTranspose, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matWorldViewProj, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matWorldViewProjTex, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&m_matTexTransform, DirectX::XMMatrixIdentity());
		}

		DirectX::XMFLOAT4X4 m_matWorld;
		DirectX::XMFLOAT4X4 m_matTexTransform;
		DirectX::XMFLOAT4X4 m_matInvTranspose;
		DirectX::XMFLOAT4X4 m_matWorldView;
		DirectX::XMFLOAT4X4 m_matWorldViewProj;
		DirectX::XMFLOAT4X4 m_matWorldViewProjTex;
	};

	struct CBPEROBJECT_PS
	{
		CBPEROBJECT_PS()
		{
			m_f3AlbedoColorMultiplier.x = m_f3AlbedoColorMultiplier.y
				= m_f3AlbedoColorMultiplier.z = 1.0f;
			m_fRoughnessMultiplier = 0.5f;
			m_fSpecularMultiplier = 0.5f;
			m_fMetallicMultiplier = 0.f;
			m_fCavityMultiplier = 0.5f;
			m_fMatMaskMultiplier = 0.f;
			m_fFlakesDensityMultiplier = 0.f;
		}

		DirectX::XMFLOAT3 m_f3AlbedoColorMultiplier;
		float m_fRoughnessMultiplier;
		float m_fSpecularMultiplier;
		float m_fMetallicMultiplier;
		float m_fCavityMultiplier;
		float m_fMatMaskMultiplier;
		float m_fFlakesDensityMultiplier;
		float padding[3];
	};

	struct CBPERFRAME_PS
	{
		float m_fNearZ;
		float m_fFarZ;
		float padding[2];
	};

	struct RENDERITEMBASE
	{
		RENDERITEMBASE()
		{
			DirectX::XMStoreFloat4x4(&m_matWorld, DirectX::XMMatrixIdentity());
		}

		DirectX::XMFLOAT4X4 m_matWorld;
		CBPEROBJECT_VS m_buffer;
	};

	struct RENDERITEM : public RENDERITEMBASE
	{
		RENDERITEM()
		{
			m_pVertBuff = NULL;
			m_pIndBuff = NULL;
			m_uIndexOffset = m_uIndexCount = m_uVertexOffset = 0;
			m_pDiffuseMap = NULL;
			m_pNormalMap = NULL;
			m_pRoughnessSpecularMetallicCavityMap = NULL;
			m_pMatMaskMap = NULL;
			m_pFlakesMap = NULL;
		}

		CBPEROBJECT_PS m_matBuffer;
		NVertexBuffer* m_pVertBuff;
		NIndexBuffer* m_pIndBuff;
		UINT32 m_uIndexOffset;
		UINT32 m_uIndexCount;
		UINT32 m_uVertexOffset;
		NTexture2DProxy* m_pDiffuseMap;
		NTexture2DProxy* m_pNormalMap;
		NTexture2DProxy* m_pRoughnessSpecularMetallicCavityMap;
		NTexture2DProxy* m_pMatMaskMap;
		NTexture2DProxy* m_pFlakesMap;
	};

	struct RENDERITEM_MODEL : public RENDERITEMBASE
	{
		RENDERITEM_MODEL()
		{
			m_bIsParameterUpdated = false;
			m_pModel = NULL;
			//m_pDiffuseMap = NULL;
			//m_pNormalMap = NULL;
			//m_pRoughnessSpecularMetallicCavityMap = NULL;
			//m_pMatMaskMap = NULL;
			//m_pFlakesMap = NULL;
		}
		BOOL m_bIsParameterUpdated;
		NGlitterModelProxy* m_pModel;
		//NTexture2DProxy* m_pDiffuseMap;
		//NTexture2DProxy* m_pNormalMap;
		//NTexture2DProxy* m_pRoughnessSpecularMetallicCavityMap;
		//NTexture2DProxy* m_pMatMaskMap;
		//NTexture2DProxy* m_pFlakesMap;
	};

	void BuildBuffers(NGraphicsDevice* dev);
	void UpdateBuffers();
	void UpdateBuffer(RENDERITEMBASE& buffer);

	void BuildGBuffers(NGraphicsDevice& dev);
	void ClearGBuffers(NRenderCmdList& list);
	void SetGBuffers(NRenderCmdList& list);
	void UnsetGBuffers(NRenderCmdList& list);

	DirectX::XMFLOAT4X4 m_matWorld;
	DirectX::XMFLOAT4X4 m_matView;
	DirectX::XMFLOAT4X4 m_matProjection;

	DirectX::XMFLOAT3 m_f3CamPos;

	NVertexBuffer m_defVertBuff;
	NIndexBuffer m_defIndBuff;

	CBPERFRAME_PS m_sPSBufferPerFrame;

	NConstantBuffer m_cPSBufferPerFrame;
	NConstantBuffer m_cBufferPerObject;
	NConstantBuffer m_cPSBufferPerObject;

	enum GBUFFER
	{
		GBUFFERS_DEPTHSTENCIL,
		GBUFFERS_NORMALDEPTHROUGHNESS,
		GBUFFERS_DIFFUSEALBEDO,
		GBUFFERS_SPECULARCAVITYMETALLICMATMASK,
		GBUFFERS_CUSTOMVARS,
		GBUFFERS_CUSTOMUINT4,
		GBUFFERS_CUSTOMUINT42,
		GBUFFERS_NO
	};
	NTexture2DRes* m_pGBuffers[GBUFFERS_NO];
	enum LIGHTACCUM
	{
		LIGHTACCUM_FIRST,
		LIGHTACCUM_SECOND,
		LIGHTACCUM_NO
	};
	NTexture2DRes* m_pLightAccumBuffers[LIGHTACCUM_NO];
	UINT32 m_u32CurrentLightAccumInd;

	std::vector<RENDERITEM> m_sRenderItems;
	std::vector<RENDERITEM_MODEL> m_sModelRenderItems;
	NTexture2DRes* m_pDefTexture;
	NTexture2DRes* m_pDefNormalTexture;
	NTexture2DRes* m_pDefFlakeTexture;

	std::shared_ptr<NResHandle> m_pVS;
	std::shared_ptr<NResHandle> m_pPS;

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

	BrdfLutMapNEffect* m_pBrdfLutMapEffect;
#ifdef GEN_PREFILTER_ENVMAP
	PrefilterEnvMapNEffect* m_pPrefilterEnvMapEffect;
#endif
#ifdef GEN_FLAKES_MAP
	GenFlakesMapNDX11Effect* m_pGenFlakesMapEffect;
#endif

	class IDeferredLight
	{
	public:
		virtual ~IDeferredLight() {}
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) = 0;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) = 0;
	};

	std::vector<IDeferredLight*> m_vLights;

	class DeferredDirLight : public IDeferredLight
	{
	public:
		DeferredDirLight(DirectX::XMFLOAT3& dir, DirectX::XMFLOAT3& color, BOOL shadowed);
		~DeferredDirLight();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VSetLightDir(DirectX::FXMVECTOR& dir);
		virtual void VSetLightColor(DirectX::FXMVECTOR& color);
		virtual void VSetShadowed(BOOL shadowed);
		virtual inline BOOL VIsShadowed() { return m_bIsShadowed; }
		virtual inline DirectX::XMVECTOR VGetLightDir() { return DirectX::XMLoadFloat3(&m_sLightBufferPerFrame.m_f3Dir); }
	protected:
		void BuildShadowBuffer(NGraphicsDevice* dev);
		void UpdateShadowBuffer();
		void UpdateBuffer(PBRNDX11Effect* effect);

		struct BOUNDINGSPHERE
		{
			BOUNDINGSPHERE() : m_vCenter(0.0f, 0.0f, 0.0f), m_fRadius(0.0f) {}
			DirectX::XMFLOAT3 m_vCenter;
			float m_fRadius;
		};

		struct SHADOW_PERFRAME
		{
			DirectX::XMFLOAT4X4 m_matLightViewProj;
		};

		struct SHADOW_PEROBJECT
		{
			DirectX::XMFLOAT4X4 m_matWorld;
			DirectX::XMFLOAT4X4 m_matTexTransform;
		};

		struct DIRLIGHT_PERFRAME
		{
			DirectX::XMFLOAT4X4 m_matLightShadowTransform;
			DirectX::XMFLOAT4X4 m_matProjection;
			DirectX::XMFLOAT3 m_f3Color;
			float m_fNearZ;
			DirectX::XMFLOAT3 m_f3Dir;
			float m_fFarZ;
			DirectX::XMFLOAT3 m_f3ViewPos;
			float padding3;
		};

		struct DIRLIGHT_VS_PERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		BOOL m_bIsShadowed;
		BOOL m_bIsNeedUpdateBuffer;
		BOOL m_bIsBuildShadowBuffer;
		BOOL m_bIsNeedUpdateShadowBuffer;

		BOUNDINGSPHERE m_sSceneBound;
		N_G_VIEWPORT m_sShadowViewport;
		NTexture2DRes* m_pShadowMap;
		SHADOW_PERFRAME m_sShadowBufferPerFrame;
		DIRLIGHT_PERFRAME m_sLightBufferPerFrame;
		DIRLIGHT_VS_PERFRAME m_sVSLightBufferPerFrame;

		static BOOL g_bIsGlobalBufferInit;
		static std::shared_ptr<NResHandle> g_pVS;
		static std::shared_ptr<NResHandle> g_pPS;
		static std::shared_ptr<NResHandle> g_pShadowVS;
		static std::shared_ptr<NResHandle> g_pShadowPS;
		static NConstantBuffer g_cVSBufferPerFrame;
		static NConstantBuffer g_cPSBufferPerFrame;
		static NConstantBuffer g_cShadowBufferPerFrame;
		static NConstantBuffer g_cShadowBufferPerObject;
		static NVertexBuffer g_screenVertBuff;
		static NIndexBuffer g_screenIndBuff;
	};

	class IPostProcessEffect
	{
	public:
		virtual ~IPostProcessEffect() {}
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) = 0;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) = 0;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) = 0;
	};

	std::vector<IPostProcessEffect*> m_vPostProcesses;

	class BasicPostProcessEffect : public IPostProcessEffect
	{
	public:
		BasicPostProcessEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override {}
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override {}

	protected:
		static BOOL g_bIsGlobalBufferInit;
		static NVertexBuffer g_screenVertBuff;
		static NIndexBuffer g_screenIndBuff;
	};

	class AmbientBRDFEffect : public BasicPostProcessEffect
	{
	public:
		AmbientBRDFEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override;

	protected:
		void UpdateBuffer(PBRNDX11Effect* effect);


		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		struct PSBUFFERPERAMBIENTCUBEMAP
		{
			DirectX::XMFLOAT4 m_ambientCubeMapColor;
			DirectX::XMFLOAT4 m_ambientCubeMipAdjust;
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;
		PSBUFFERPERAMBIENTCUBEMAP m_sPSBufferPerAmbientCubeMap;
		NConstantBuffer m_cPSBufferPerAmbientCubeMap;

		std::shared_ptr<NResHandle> m_pBrdfLutMap;
		std::shared_ptr<NResHandle> m_pAmbientCubeMap;
	};

	class CopyToBackBufferEffect : public BasicPostProcessEffect
	{
	public:
		CopyToBackBufferEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override {}

	protected:
		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;
	};

	class BasicSkyEffect : public BasicPostProcessEffect
	{
	public:
		BasicSkyEffect(std::shared_ptr<NResHandle> cubeMap);
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override;

	protected:
		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;

		std::shared_ptr<NResHandle> m_pCubeMap;
	};

	class GlitterEffect : public BasicPostProcessEffect
	{
	public:
		GlitterEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override;

	protected:
		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		struct PSBUFFERPERFRAME
		{
			float m_fMaskMat;
			float padding[3];
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;
		PSBUFFERPERFRAME m_sPSBufferPerFrame;
		NConstantBuffer m_cPSBufferPerFrame;

		std::shared_ptr<NResHandle> m_pNormalMap;
		std::shared_ptr<NResHandle> m_pFlakesMap;
	};

	class SSAOEffect : public BasicPostProcessEffect
	{
	public:
		SSAOEffect();
		~SSAOEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override;

		inline NTexture2DRes* GetAmbientMap() { return m_pAmbientMap[0]; }
	protected:
		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		struct PSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matViewToTexSpace;
			float m_fFarZ;
			float padding[3];
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;
		PSBUFFERPERFRAME m_sPSBufferPerFrame;
		NConstantBuffer m_cPSBufferPerFrame;

		N_G_VIEWPORT m_ssaoViewport;

		NTexture2DRes* m_pAmbientMap[2];

		std::shared_ptr<NResHandle> m_pBlurPS;

		struct BLURPSBUFFERPERFRAME
		{
			DirectX::XMFLOAT2 m_vOffsetSize;
			float m_fFarZ;
			float padding[1];
		};
		BLURPSBUFFERPERFRAME m_sBlurPSBufferPerFrame[2];
		NConstantBuffer m_cBlurPSBufferPerFrame;
	};

	class CompositeEffect : public BasicPostProcessEffect
	{
	public:
		CompositeEffect(SSAOEffect* ssao);
		~CompositeEffect();
		virtual void VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache) override;
		virtual void VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect) override;
		virtual void VUpdateBuffer(PBRNDX11Effect* effect) override;

	protected:
		struct VSBUFFERPERFRAME
		{
			DirectX::XMFLOAT4X4 m_matWorldViewProj;
			DirectX::XMFLOAT4X4 m_matScreenToTranslatedWorld;
		};

		std::shared_ptr<NResHandle> m_pVS;
		std::shared_ptr<NResHandle> m_pPS;

		VSBUFFERPERFRAME m_sVSBufferPerFrame;
		NConstantBuffer m_cVSBufferPerFrame;

		SSAOEffect* m_pSSAO;
	};
};