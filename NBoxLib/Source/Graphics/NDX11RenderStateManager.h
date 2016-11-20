#pragma once
#include "NDX11.h"
#include "NDX11RenderState.h"

class NDX11RenderStateManager
{
public:
	NDX11RenderStateManager()
	{}

	void InitAll(NDX11& dev)
	{
		//
		// WireframeRS
		//
		N_G_RASTERIZER_DESC wireframeDesc;
		ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
		wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireframeDesc.CullMode = D3D11_CULL_BACK;
		wireframeDesc.FrontCounterClockwise = false;
		wireframeDesc.DepthClipEnable = true;
		dev.CreateRasterizerState(m_wireframeRS, wireframeDesc);

		//
		// NoCullRS
		//
		N_G_RASTERIZER_DESC noCullDesc;
		ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
		noCullDesc.FillMode = D3D11_FILL_SOLID;
		noCullDesc.CullMode = D3D11_CULL_NONE;
		noCullDesc.FrontCounterClockwise = false;
		noCullDesc.DepthClipEnable = true;
		dev.CreateRasterizerState(m_noCullRS, noCullDesc);

		//
		// DepthRS
		//
		N_G_RASTERIZER_DESC DepthDesc;
		ZeroMemory(&DepthDesc, sizeof(D3D11_RASTERIZER_DESC));
		DepthDesc.FillMode = D3D11_FILL_SOLID;
		DepthDesc.CullMode = D3D11_CULL_BACK;
		DepthDesc.FrontCounterClockwise = false;
		DepthDesc.DepthClipEnable = true;
		DepthDesc.DepthBias = 10000;
		DepthDesc.DepthBiasClamp = 0;
		DepthDesc.SlopeScaledDepthBias = 0.5f;
		dev.CreateRasterizerState(m_depthRS, DepthDesc);

		//
		// EqualsDSS
		//
		N_G_DEPTHSTENCIL_DESC equalsDesc;
		ZeroMemory(&equalsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		equalsDesc.DepthEnable = true;
		equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		equalsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
		dev.CreateDepthStencilState(m_equalsDSS, equalsDesc);

		//
		// LessEqualsDSS
		//
		N_G_DEPTHSTENCIL_DESC lessEqualsDesc;
		ZeroMemory(&lessEqualsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		lessEqualsDesc.DepthEnable = true;
		lessEqualsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		lessEqualsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dev.CreateDepthStencilState(m_lessEqualsDSS, lessEqualsDesc);

		///
		/// m_PostProcessDSS
		///
		N_G_DEPTHSTENCIL_DESC postProcessDesc;
		ZeroMemory(&postProcessDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		postProcessDesc.DepthEnable = false;
		equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		equalsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		dev.CreateDepthStencilState(m_postProcessDSS, postProcessDesc);

		//
		// AlphaToCoverageBS
		//
		D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
		alphaToCoverageDesc.AlphaToCoverageEnable = true;
		alphaToCoverageDesc.IndependentBlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
		alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dev.CreateBlendStateState(m_alphaToCoverageBS, alphaToCoverageDesc);

		//
		// TransparentBS
		//
		D3D11_BLEND_DESC transparentDesc = { 0 };
		transparentDesc.AlphaToCoverageEnable = false;
		transparentDesc.IndependentBlendEnable = false;
		transparentDesc.RenderTarget[0].BlendEnable = true;
		transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dev.CreateBlendStateState(m_transparentBS, transparentDesc);

		//
		// ParticleBS
		//
		D3D11_BLEND_DESC particleDesc = { 0 };
		particleDesc.AlphaToCoverageEnable = false;
		particleDesc.IndependentBlendEnable = false;
		particleDesc.RenderTarget[0].BlendEnable = true;
		particleDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		particleDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		particleDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		particleDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		particleDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		particleDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		particleDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dev.CreateBlendStateState(m_particleBS, particleDesc);

		///
		/// Deferred Light Accumulation BS
		///
		D3D11_BLEND_DESC deferredLightAccumDesc = { 0 };
		deferredLightAccumDesc.AlphaToCoverageEnable = false;
		deferredLightAccumDesc.IndependentBlendEnable = false;
		deferredLightAccumDesc.RenderTarget[0].BlendEnable = true;
		deferredLightAccumDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		deferredLightAccumDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		deferredLightAccumDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		deferredLightAccumDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		deferredLightAccumDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		deferredLightAccumDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		deferredLightAccumDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dev.CreateBlendStateState(m_deferredLightAccumBS, particleDesc);

		N_G_SAMPLER_DESC linearSamplerDesc;
		ZeroMemory(&linearSamplerDesc, sizeof(linearSamplerDesc));
		linearSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		linearSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		linearSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		linearSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		linearSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		linearSamplerDesc.MinLOD = 0;
		linearSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		dev.CreateSamplersState(m_linearSampler, linearSamplerDesc);

		N_G_SAMPLER_DESC shadowSamplerDesc;
		ZeroMemory(&shadowSamplerDesc, sizeof(shadowSamplerDesc));
		shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		shadowSamplerDesc.MinLOD = 0;
		shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		dev.CreateSamplersState(m_shadowSampler, shadowSamplerDesc);

		N_G_SAMPLER_DESC discreteSamplerDesc;
		ZeroMemory(&discreteSamplerDesc, sizeof(discreteSamplerDesc));
		discreteSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		discreteSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		discreteSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		discreteSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		discreteSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		discreteSamplerDesc.MinLOD = 0;
		discreteSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		dev.CreateSamplersState(m_discreteSampler, discreteSamplerDesc);

		N_G_SAMPLER_DESC blurSamplerDesc;
		ZeroMemory(&blurSamplerDesc, sizeof(blurSamplerDesc));
		blurSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		blurSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		blurSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		blurSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		blurSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		blurSamplerDesc.MinLOD = 0;
		blurSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		dev.CreateSamplersState(m_blurSampler, linearSamplerDesc);
	}

	void DestroyAll()
	{

	}

public:
	NDX11RasterizerState m_wireframeRS;
	NDX11RasterizerState m_noCullRS;
	NDX11RasterizerState m_depthRS;

	NDX11DepthStencilState m_equalsDSS;
	NDX11DepthStencilState m_lessEqualsDSS;
	NDX11DepthStencilState m_postProcessDSS;

	NDX11BlendState m_alphaToCoverageBS;
	NDX11BlendState m_transparentBS;
	NDX11BlendState m_particleBS;
	NDX11BlendState m_deferredLightAccumBS;

	NDX11SamplersState m_linearSampler;
	NDX11SamplersState m_shadowSampler;
	NDX11SamplersState m_discreteSampler;
	NDX11SamplersState m_blurSampler;
};