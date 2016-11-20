#pragma once

#include "../Macro/Macro.h"

#ifdef N_G_DIRECTX_11
#include "NDX11.h"
#include "NDX11Buffer.h"
#include "NDX11RenderTargetManager.h"
#include "NDX11RenderStateManager.h"
#include "NDX11RenderState.h"
#include "NDX11Effect.h"
#include "NDX11Shader.h"
#include "NTexture2DPool.h"

typedef NDX11 NGraphicsDevice;
typedef NDX11Buffer NBuffer;
typedef NDX11VertexBuffer NVertexBuffer;
typedef NDX11IndexBuffer NIndexBuffer;
typedef NDX11ConstantBuffer NConstantBuffer;
typedef NDX11StructureBuffer NStructureBuffer;
typedef NDX11RenderTargetManager NRenderTargetManager;

typedef NDX11DepthStencilState NDepthStencilState;
typedef NDX11RasterizerState NRasterizerState;
typedef NDX11BlendState NBlendState;
typedef NDX11SamplersState NSamplersState;

typedef NDX11VertexShader NVertexShader;
typedef NDX11PixelShader NPixelShader;
typedef NDX11GeometryShader NGeometryShader;
typedef NDX11ComputeShader NComputeShader;
typedef NDX11Texture2DRes NTexture2DRes;

typedef INDX11Effect INEffect;
typedef INDX11EffectMatrices INEffectMatrices;
typedef INDX11EffectDirLights INEffectDirLights;
typedef INDX11EffectFog INEffectFog;
#endif