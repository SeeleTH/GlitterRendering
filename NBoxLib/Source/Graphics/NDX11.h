#pragma once
#include "../Macro/Macro.h"
#include "../Debug/NAssert.h"
#include "../Thread/NCriticalSection.h"

#pragma warning(push)
#pragma warning (disable: 4005)
#include <D3DX11.h>
#include <DxErr.h>
#include <d3dcompiler.h>
#pragma warning(pop)

#if defined(DEBUG) | defined(_DEBUG)
#ifndef DXHR
#define DXHR(x)                                              \
        {                                                          \
    HRESULT hr0 = (x);                                      \
if (FAILED(hr0))                                         \
        {                                                      \
        DXTrace(__FILE__, (DWORD)__LINE__, hr0, L#x, true); \
        }                                                      \
        }
#endif
#else
#ifndef DXHR
#define DXHR(x) (x)
#endif
#endif

enum N_BUFFERS {
	N_B_COLOR_BUFFER = (1UL << 16UL),
	N_B_STENCIL_BUFFER = D3D11_CLEAR_STENCIL,
	N_B_DEPTH_BUFFER = D3D11_CLEAR_DEPTH,
	N_B_ALL = N_B_COLOR_BUFFER | N_B_STENCIL_BUFFER | N_B_DEPTH_BUFFER,
};

enum N_BUFFER_USAGE
{
	N_B_USAGE_DEFAULT = 0,
	N_B_USAGE_IMMUTABLE = 1,
	N_B_USAGE_DYNAMIC = 2,
	N_B_USAGE_STAGING = 3
};

enum N_RESOURCE_FORMAT
{
	N_R_FORMAT_UNKNOWN = 0,
	N_R_FORMAT_R32G32B32A32_TYPELESS = 1,
	N_R_FORMAT_R32G32B32A32_FLOAT = 2,
	N_R_FORMAT_R32G32B32A32_UINT = 3,
	N_R_FORMAT_R32G32B32A32_SINT = 4,
	N_R_FORMAT_R32G32B32_TYPELESS = 5,
	N_R_FORMAT_R32G32B32_FLOAT = 6,
	N_R_FORMAT_R32G32B32_UINT = 7,
	N_R_FORMAT_R32G32B32_SINT = 8,
	N_R_FORMAT_R16G16B16A16_TYPELESS = 9,
	N_R_FORMAT_R16G16B16A16_FLOAT = 10,
	N_R_FORMAT_R16G16B16A16_UNORM = 11,
	N_R_FORMAT_R16G16B16A16_UINT = 12,
	N_R_FORMAT_R16G16B16A16_SNORM = 13,
	N_R_FORMAT_R16G16B16A16_SINT = 14,
	N_R_FORMAT_R32G32_TYPELESS = 15,
	N_R_FORMAT_R32G32_FLOAT = 16,
	N_R_FORMAT_R32G32_UINT = 17,
	N_R_FORMAT_R32G32_SINT = 18,
	N_R_FORMAT_R32G8X24_TYPELESS = 19,
	N_R_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	N_R_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	N_R_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	N_R_FORMAT_R10G10B10A2_TYPELESS = 23,
	N_R_FORMAT_R10G10B10A2_UNORM = 24,
	N_R_FORMAT_R10G10B10A2_UINT = 25,
	N_R_FORMAT_R11G11B10_FLOAT = 26,
	N_R_FORMAT_R8G8B8A8_TYPELESS = 27,
	N_R_FORMAT_R8G8B8A8_UNORM = 28,
	N_R_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	N_R_FORMAT_R8G8B8A8_UINT = 30,
	N_R_FORMAT_R8G8B8A8_SNORM = 31,
	N_R_FORMAT_R8G8B8A8_SINT = 32,
	N_R_FORMAT_R16G16_TYPELESS = 33,
	N_R_FORMAT_R16G16_FLOAT = 34,
	N_R_FORMAT_R16G16_UNORM = 35,
	N_R_FORMAT_R16G16_UINT = 36,
	N_R_FORMAT_R16G16_SNORM = 37,
	N_R_FORMAT_R16G16_SINT = 38,
	N_R_FORMAT_R32_TYPELESS = 39,
	N_R_FORMAT_D32_FLOAT = 40,
	N_R_FORMAT_R32_FLOAT = 41,
	N_R_FORMAT_R32_UINT = 42,
	N_R_FORMAT_R32_SINT = 43,
	N_R_FORMAT_R24G8_TYPELESS = 44,
	N_R_FORMAT_D24_UNORM_S8_UINT = 45,
	N_R_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	N_R_FORMAT_X24_TYPELESS_G8_UINT = 47,
	N_R_FORMAT_R8G8_TYPELESS = 48,
	N_R_FORMAT_R8G8_UNORM = 49,
	N_R_FORMAT_R8G8_UINT = 50,
	N_R_FORMAT_R8G8_SNORM = 51,
	N_R_FORMAT_R8G8_SINT = 52,
	N_R_FORMAT_R16_TYPELESS = 53,
	N_R_FORMAT_R16_FLOAT = 54,
	N_R_FORMAT_D16_UNORM = 55,
	N_R_FORMAT_R16_UNORM = 56,
	N_R_FORMAT_R16_UINT = 57,
	N_R_FORMAT_R16_SNORM = 58,
	N_R_FORMAT_R16_SINT = 59,
	N_R_FORMAT_R8_TYPELESS = 60,
	N_R_FORMAT_R8_UNORM = 61,
	N_R_FORMAT_R8_UINT = 62,
	N_R_FORMAT_R8_SNORM = 63,
	N_R_FORMAT_R8_SINT = 64,
	N_R_FORMAT_A8_UNORM = 65,
	N_R_FORMAT_R1_UNORM = 66,
	N_R_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	N_R_FORMAT_R8G8_B8G8_UNORM = 68,
	N_R_FORMAT_G8R8_G8B8_UNORM = 69,
	N_R_FORMAT_BC1_TYPELESS = 70,
	N_R_FORMAT_BC1_UNORM = 71,
	N_R_FORMAT_BC1_UNORM_SRGB = 72,
	N_R_FORMAT_BC2_TYPELESS = 73,
	N_R_FORMAT_BC2_UNORM = 74,
	N_R_FORMAT_BC2_UNORM_SRGB = 75,
	N_R_FORMAT_BC3_TYPELESS = 76,
	N_R_FORMAT_BC3_UNORM = 77,
	N_R_FORMAT_BC3_UNORM_SRGB = 78,
	N_R_FORMAT_BC4_TYPELESS = 79,
	N_R_FORMAT_BC4_UNORM = 80,
	N_R_FORMAT_BC4_SNORM = 81,
	N_R_FORMAT_BC5_TYPELESS = 82,
	N_R_FORMAT_BC5_UNORM = 83,
	N_R_FORMAT_BC5_SNORM = 84,
	N_R_FORMAT_B5G6R5_UNORM = 85,
	N_R_FORMAT_B5G5R5A1_UNORM = 86,
	N_R_FORMAT_B8G8R8A8_UNORM = 87,
	N_R_FORMAT_B8G8R8X8_UNORM = 88,
	N_R_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	N_R_FORMAT_B8G8R8A8_TYPELESS = 90,
	N_R_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	N_R_FORMAT_B8G8R8X8_TYPELESS = 92,
	N_R_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	N_R_FORMAT_BC6H_TYPELESS = 94,
	N_R_FORMAT_BC6H_UF16 = 95,
	N_R_FORMAT_BC6H_SF16 = 96,
	N_R_FORMAT_BC7_TYPELESS = 97,
	N_R_FORMAT_BC7_UNORM = 98,
	N_R_FORMAT_BC7_UNORM_SRGB = 99,
	N_R_FORMAT_FORCE_UINT = 0xffffffff
};

enum N_DSV_DIMENSION
{
	N_DSV_DIMENSION_UNKNOWN = 0,
	N_DSV_DIMENSION_TEXTURE1D = 1,
	N_DSV_DIMENSION_TEXTURE1DARRAY = 2,
	N_DSV_DIMENSION_TEXTURE2D = 3,
	N_DSV_DIMENSION_TEXTURE2DARRAY = 4,
	N_DSV_DIMENSION_TEXTURE2DMS = 5,
	N_DSV_DIMENSION_TEXTURE2DMSARRAY = 6
};

enum N_RESOURCE_DIMENSION
{
	N_RESOURCE_DIMENSION_UNKNOWN = 0,
	N_RESOURCE_DIMENSION_BUFFER = 1,
	N_RESOURCE_DIMENSION_TEXTURE1D = 2,
	N_RESOURCE_DIMENSION_TEXTURE2D = 3,
	N_RESOURCE_DIMENSION_TEXTURE3D = 4
};

enum N_RTV_DIMENSION
{
	N_RTV_DIMENSION_UNKNOWN = 0,
	N_RTV_DIMENSION_BUFFER = 1,
	N_RTV_DIMENSION_TEXTURE1D = 2,
	N_RTV_DIMENSION_TEXTURE1DARRAY = 3,
	N_RTV_DIMENSION_TEXTURE2D = 4,
	N_RTV_DIMENSION_TEXTURE2DARRAY = 5,
	N_RTV_DIMENSION_TEXTURE2DMS = 6,
	N_RTV_DIMENSION_TEXTURE2DMSARRAY = 7,
	N_RTV_DIMENSION_TEXTURE3D = 8
};

typedef struct N_G_COMPILE_SHADER_INFO
{
	std::string ShaderName;
	char* RawBuffer;
	UINT32 RawSize;
	LPCSTR EntryPoint;
	LPCSTR ShaderModel;
	ID3DBlob* BlobOut;
} N_G_COMPILE_SHADER_INFO;

#define SHADER_LAYOUT_MAX_ELEMENTS 10
typedef struct N_G_SHADER_LAYOUT
{
	D3D11_INPUT_ELEMENT_DESC layouts[SHADER_LAYOUT_MAX_ELEMENTS];
	UINT32 numElements;
} N_G_SHADER_LAYOUT;

typedef D3D11_TEXTURE2D_DESC				N_G_TEXTURE2D_DESC;
typedef D3D11_DEPTH_STENCIL_VIEW_DESC		N_G_DEPTH_STENCIL_VIEW_DESC;
typedef D3D11_SHADER_RESOURCE_VIEW_DESC		N_G_SHADER_RESOURCE_VIEW_DESC;
typedef D3D11_UNORDERED_ACCESS_VIEW_DESC	N_G_UNORDERED_ACCESS_VIEW_DESC;
typedef D3D11_RENDER_TARGET_VIEW_DESC		N_G_RENDER_TARGET_VIEW_DESC;
typedef D3D11_DEPTH_STENCIL_DESC			N_G_DEPTHSTENCIL_DESC;
typedef D3D11_RASTERIZER_DESC				N_G_RASTERIZER_DESC;
typedef D3D11_BLEND_DESC					N_G_BLEND_DESC;
typedef D3D11_SAMPLER_DESC					N_G_SAMPLER_DESC;

typedef D3D11_VIEWPORT N_G_VIEWPORT;

class NDX11RenderStateManager;

class NDX11DepthStencilState;
class NDX11RasterizerState;
class NDX11BlendState;
class NDX11SamplersState;

class NDX11VertexShader;
class NDX11PixelShader;
class NDX11GeometryShader;
class NDX11ComputeShader;

class NDX11VertexBuffer;
class NDX11IndexBuffer;
class NDX11ConstantBuffer;
class NDX11StructureBuffer;

class NDX11Texture2DRes;

class NDX11
{
public:
	static NDX11* g_pMainGraphicsDevice;

    NDX11();
    ~NDX11();

    BOOL PreInit();
    BOOL Init(HWND hWnd, int clientWidth, int clientHeight);
    VOID Destroy();
    VOID OnResize(UINT32 width, UINT32 height);

	inline void Present();
	inline void Clear(UINT32 ui32Mask = N_B_ALL, ID3D11RenderTargetView* rtv = nullptr
		, ID3D11DepthStencilView* dsv = nullptr, float depth = -1.f, UINT8 stencil = 0
		, float colorR = -1.f, float colorG = -1.f, float colorB = -1.f, float colorA = -1.f);
	inline void SetRenderTargets(ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, UINT32 rtvNumber);
	inline void SetViewports(N_G_VIEWPORT* viewports, UINT32 vpNumber);

	inline UINT32 GetClientWidth() { return m_iClientWidth; }
	inline UINT32 GetClientHeight() { return m_iClientHeight; }
    inline ID3D11Device* GetD3D11Device() { return m_pD3dDevice; }
    inline ID3D11DeviceContext* GetD3D11DeviceContext() { return m_pD3dImmediateContext; }
	inline N_G_VIEWPORT* GetScreenViewport() { return (N_G_VIEWPORT*)&m_sScreenViewport; }
	ID3D11DepthStencilView* GetDepthStencilView();
	ID3D11RenderTargetView* GetRenderTargetView();

	inline NDX11RenderStateManager* GetRenderStateManager() { return m_pRenderStateManager; }

	// Shaders
	NDX11VertexShader CompileAndCreateVertexShader(N_G_COMPILE_SHADER_INFO &compileInfo, N_G_SHADER_LAYOUT &layout);
	NDX11PixelShader CompileAndCreatePixelShader(N_G_COMPILE_SHADER_INFO &compileInfo);
	NDX11GeometryShader CompileAndCreateGeometryShader(N_G_COMPILE_SHADER_INFO &compileInfo);
	NDX11ComputeShader CompileAndCreateComputeShader(N_G_COMPILE_SHADER_INFO &compileInfo);
	NDX11VertexShader CreateVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength);
	NDX11VertexShader CreateVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength, N_G_SHADER_LAYOUT &layout);
	NDX11PixelShader CreatePixelShader(const void* shaderByteCode, SIZE_T byteCodeLength);
	NDX11GeometryShader CreateGeometryShader(const void* shaderByteCode, SIZE_T byteCodeLength);
	NDX11ComputeShader CreateComputeShader(const void* shaderByteCode, SIZE_T byteCodeLength);
	void SetVertexShaderWLayout(NDX11VertexShader* shader);
	void SetPixelShader(NDX11PixelShader* shader);
	void SetGeometryShader(NDX11GeometryShader* shader);
	void SetComputeShader(NDX11ComputeShader* shader);

	// Buffers
	void CreateVertexBuffer(NDX11VertexBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
		, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount, UINT32 stride, UINT32 offset);
	void CreateIndexBuffer(NDX11IndexBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
		, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount, N_RESOURCE_FORMAT format, UINT32 offset);
	void CreateConstantBuffer(NDX11ConstantBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize, UINT32 dataCount
		, UINT32 cpuAccFlag, UINT32 miscFlag);
	void CreateStructureBuffer(NDX11StructureBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
		, UINT32 bindFlag, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount);
	void* MapConstantBuffer(NDX11ConstantBuffer& buffer);
	void UnMapConstantBuffer(NDX11ConstantBuffer& buffer);
	void SetConstantBufferData(NDX11ConstantBuffer& buffer, void* value, SIZE_T size);
	void SetVSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer);
	void SetPSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer);
	void SetGSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer);
	void SetCSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer);
	void SetVSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures);
	void SetPSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures);
	void SetGSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures);
	void SetCSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures);
	void SetVSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers);
	void SetPSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers);
	void SetGSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers);
	void SetCSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers);
	void SetCSUnoderedAccessViews(UINT32 startSlot, UINT32 numViews, NDX11Texture2DRes** textures);
	void SetCSUnoderedAccessViews(UINT32 startSlot, UINT32 numViews, NDX11StructureBuffer** buffers);
	void BindVertexBuffer(UINT32 startSlot, UINT32 numBuffers, NDX11VertexBuffer* buffer);
	void BindIndexBuffer(NDX11IndexBuffer* buffer);
	void SetPrimitiveTopology(UINT32 topology);

	// Draw
	void DrawIndexed(UINT32 indexCount, UINT32 indexOffset, UINT32 vertexOffset);
	void Draw(UINT32 vertexCount, UINT32 vertexOffset);
	void Dispatch(UINT32 threadGroupCountX, UINT32 threadGroupCountY, UINT32 threadGroupCountZ);

	// Texture
	void CreateTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size);
	void CreateTextureFromDesc(NDX11Texture2DRes& texture, N_G_TEXTURE2D_DESC &desc, UINTPTR initData, UINT32 dataSize);
	void CreateDSSTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size);
	void WriteTextureToDDS(NDX11Texture2DRes& texture, std::string filepath, N_RESOURCE_FORMAT format = N_R_FORMAT_UNKNOWN);
	void CreateWICTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size);

	// DepthStencilView
	void CreateDSV(NDX11Texture2DRes& texture);
	void CreateDSV(NDX11Texture2DRes& texture, N_G_DEPTH_STENCIL_VIEW_DESC& desc);
	void ClearDSV(NDX11Texture2DRes& texture, UINT32 clearFlags, float depth, UINT8 stencil);

	// ShaderResourceView
	void CreateSRV(NDX11Texture2DRes& texture);
	void CreateSRV(NDX11Texture2DRes& texture, N_G_SHADER_RESOURCE_VIEW_DESC& desc);
	void CreateSRV(NDX11StructureBuffer& buffer, N_G_SHADER_RESOURCE_VIEW_DESC& desc);

	// UnorderAccessView
	void CreateUAV(NDX11Texture2DRes& texture);
	void CreateUAV(NDX11Texture2DRes& texture, N_G_UNORDERED_ACCESS_VIEW_DESC& desc);
	void CreateUAV(NDX11StructureBuffer& buffer, N_G_UNORDERED_ACCESS_VIEW_DESC& desc);

	// RenderTargets
	void CreateRTV(NDX11Texture2DRes& texture);
	void CreateRTVFromDesc(NDX11Texture2DRes& texture, N_G_RENDER_TARGET_VIEW_DESC& desc);
	void ClearRTV(NDX11Texture2DRes& texture, float color[4]);
	void SetRenderTargets(NDX11Texture2DRes** rtvs, UINT32 rtvsNum, NDX11Texture2DRes* dsv);
	inline NDX11Texture2DRes* GetMainRenderTargets() { return m_pMainRenderTarget; }
	inline NDX11Texture2DRes* GetMainDepthStencil() { return m_pMainDepthStencil; }

	// RenderState
	void CreateDepthStencilState(NDX11DepthStencilState& state, N_G_DEPTHSTENCIL_DESC& desc);
	void CreateRasterizerState(NDX11RasterizerState& state, N_G_RASTERIZER_DESC& desc);
	void CreateBlendStateState(NDX11BlendState& state, N_G_BLEND_DESC& desc);
	void CreateSamplersState(NDX11SamplersState& state, N_G_SAMPLER_DESC& desc);
	void SetDepthStencilState(NDX11DepthStencilState* state, UINT32 stencilRef);
	void SetRasterizerState(NDX11RasterizerState* state);
	void SetBlendStateState(NDX11BlendState* state, float blendFactors[4], UINT32 sampleMask);
	void SetPSSamplersState(NDX11SamplersState* state, UINT32 startSlot, UINT32 numSamplers);
	void SetCSSamplersState(NDX11SamplersState* state, UINT32 startSlot, UINT32 numSamplers);

protected:
	void CompileHLSLShaderFromMemory(N_G_COMPILE_SHADER_INFO &compileInfo);
	void CreateHLSLVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11VertexShader** result);
	void CreateHLSLPixelShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11PixelShader** result);
	void CreateHLSLGeometryShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11GeometryShader** result);
	void CreateHLSLComputeShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11ComputeShader** result);
	void CreateHLSLInputLayout(D3D11_INPUT_ELEMENT_DESC layout[], UINT32 numElements, const void* byteCode, SIZE_T byteCodeLength, ID3D11InputLayout** result);

	void CreateBuffer(ID3D11Buffer** buffer, N_BUFFER_USAGE usage, UINT32 structSize, UINT32 bingFlag
		, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount);

protected:
    HWND m_pHMainWnd;
    UINT32 m_iClientWidth;
    UINT32 m_iClientHeight;

    D3D_DRIVER_TYPE m_sD3dDriverType;
    ID3D11Device* m_pD3dDevice;
    ID3D11DeviceContext* m_pD3dImmediateContext;
    IDXGISwapChain* m_pSwapChain;

	D3D11_VIEWPORT m_sScreenViewport;

	NDX11Texture2DRes* m_pMainRenderTarget;
	NDX11Texture2DRes* m_pMainDepthStencil;

    BOOL m_bEnable4xMsaa;
    UINT32 m_u4xMsaaQuality;

    UINT32 m_uiClearColor;
    float m_fDepthClear;

	NDX11RenderStateManager* m_pRenderStateManager;

	NCriticalSection m_csDeviceLock;
};


void NDX11::Present()
{
    N_ASSERT(m_pSwapChain);
    DXHR(m_pSwapChain->Present(0, 0));
}

void NDX11::Clear(UINT32 ui32Mask, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, float depth, UINT8 stencil, float colorR, float colorG, float colorB, float colorA)
{
	N_ASSERT(m_pD3dImmediateContext);
	if (ui32Mask & N_B_COLOR_BUFFER && rtv) 
	{
		// Clearing color.
		float color[4] = { colorR, colorG, colorB, colorA };
		if (color[0] < 0.f) color[0] = (m_uiClearColor >> 24) * (1.0f / 255.0f);
		if (color[1] < 0.f) color[1] = ((m_uiClearColor >> 16) & 0xFF) * (1.0f / 255.0f);
		if (color[2] < 0.f) color[2] = ((m_uiClearColor >> 8) & 0xFF) * (1.0f / 255.0f);
		if (color[3] < 0.f) color[3] = (m_uiClearColor & 0xFF) * (1.0f / 255.0f);
		m_pD3dImmediateContext->ClearRenderTargetView((rtv) ? rtv : GetRenderTargetView(), color);

	}

	if (dsv)
	{
		m_pD3dImmediateContext->ClearDepthStencilView((dsv) ? dsv : GetDepthStencilView(), ui32Mask & (N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER),
			(depth >= 0) ? depth : m_fDepthClear, stencil);
	}
}

void NDX11::SetRenderTargets(ID3D11RenderTargetView** rtvs, ID3D11DepthStencilView* dsv, UINT32 rtvNumber)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->OMSetRenderTargets(rtvNumber, rtvs, dsv);
}

void NDX11::SetViewports(N_G_VIEWPORT* viewports, UINT32 vpNumber)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->RSSetViewports(vpNumber, (D3D11_VIEWPORT*)viewports);
}

typedef NDX11 NGraphicsDevice;