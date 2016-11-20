#include "../3rdParty/DirectXTK/Inc/DDSTextureLoader.h"
#include "../3rdParty/DirectXTK/Inc/WICTextureLoader.h"

#include "NDX11.h"

#include "NDX11RenderStateManager.h"
#include "NDX11Shader.h"
#include "NDX11Buffer.h"
#include "NDX11RenderState.h"
#include "NTexture2DRes.h"

#include "../3rdParty/DirectXTex/DirectXTex/DirectXTex.h"

NDX11* NDX11::g_pMainGraphicsDevice = NULL;

NDX11::NDX11()
: m_pHMainWnd(0)
, m_iClientWidth(0)
, m_iClientHeight(0)
, m_sD3dDriverType(D3D_DRIVER_TYPE_HARDWARE)
, m_pD3dDevice(0)
, m_pD3dImmediateContext(0)
, m_pSwapChain(0)
, m_sScreenViewport()
, m_pMainRenderTarget(0)
, m_pMainDepthStencil(0)
, m_bEnable4xMsaa(false)
, m_u4xMsaaQuality(0)
, m_uiClearColor(0)
, m_fDepthClear(1.0f)
, m_pRenderStateManager(0)
, m_csDeviceLock()
{

}

NDX11::~NDX11()
{
    Destroy();
}

BOOL NDX11::PreInit()
{
    ZeroMemory(&m_sScreenViewport, sizeof(D3D11_VIEWPORT));

    return true;
}

BOOL NDX11::Init(HWND hWnd, int clientWidth, int clientHeight)
{
	g_pMainGraphicsDevice = this;
    m_pHMainWnd = hWnd;
    m_iClientWidth = clientWidth;
    m_iClientHeight = clientHeight;
	m_pMainRenderTarget = new NDX11Texture2DRes();
	m_pMainDepthStencil = new NDX11Texture2DRes();

    // Create the device and device context.
    INT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        0,                 // default adapter
        m_sD3dDriverType,
        0,                 // no software device
        createDeviceFlags,
        0, 0,              // default feature level array
        D3D11_SDK_VERSION,
        &m_pD3dDevice,
        &featureLevel,
        &m_pD3dImmediateContext);

    if (FAILED(hr))
    {
        MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
        return false;
    }

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render 
    // target formats, so we only need to check quality support.

    DXHR(m_pD3dDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_u4xMsaaQuality));
    assert(m_u4xMsaaQuality > 0);

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = clientWidth;
    sd.BufferDesc.Height = clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Use 4X MSAA? 
    if (m_bEnable4xMsaa)
    {
        sd.SampleDesc.Count = 4;
        sd.SampleDesc.Quality = m_u4xMsaaQuality - 1;
    }
    // No MSAA
    else
    {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    // To correctly create the swap chain, we must use the IDXGIFactory that was
    // used to create the device.  If we tried to use a different IDXGIFactory instance
    // (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
    // This function is being called with a device from a different IDXGIFactory."

    IDXGIDevice* dxgiDevice = 0;
    DXHR(m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

    IDXGIAdapter* dxgiAdapter = 0;
    DXHR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

    IDXGIFactory* dxgiFactory = 0;
    DXHR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

    DXHR(dxgiFactory->CreateSwapChain(m_pD3dDevice, &sd, &m_pSwapChain));

    N_RELEASE(dxgiDevice);
    N_RELEASE(dxgiAdapter);
    N_RELEASE(dxgiFactory);

    OnResize(clientWidth, clientHeight);

	m_pRenderStateManager = new NDX11RenderStateManager();
	m_pRenderStateManager->InitAll(*this);

    return true;
}

void NDX11::Destroy()
{
	if (m_pRenderStateManager)
		m_pRenderStateManager->DestroyAll();
	N_DELETE(m_pRenderStateManager);

	N_DELETE(m_pMainRenderTarget);
	N_DELETE(m_pMainDepthStencil);


    // Restore all default settings.
    if (m_pD3dImmediateContext)
        m_pD3dImmediateContext->ClearState();

    N_RELEASE(m_pD3dImmediateContext);
    N_RELEASE(m_pD3dDevice);
}

void NDX11::OnResize(UINT32 width, UINT32 height)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);

    m_iClientWidth = width;
    m_iClientHeight = height;

    assert(m_pD3dImmediateContext);
    assert(m_pD3dDevice);
    assert(m_pSwapChain);

    // Release the old views, as they hold references to the buffers we
    // will be destroying.  Also release the old depth/stencil buffer.
	N_DELETE(m_pMainRenderTarget->m_pRTV);
	N_DELETE(m_pMainDepthStencil->m_pDSV);
	N_DELETE(m_pMainDepthStencil->m_pTexture);


    // Resize the swap chain and recreate the render target view.
    DXHR(m_pSwapChain->ResizeBuffers(1, m_iClientWidth, m_iClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    ID3D11Texture2D* backBuffer;
    DXHR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	DXHR(m_pD3dDevice->CreateRenderTargetView(backBuffer, 0, &m_pMainRenderTarget->m_pRTV));

    // Create the depth/stencil buffer and view.
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = m_iClientWidth;
    depthStencilDesc.Height = m_iClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // Use 4X MSAA? --must match swap chain MSAA values.
    if (m_bEnable4xMsaa)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m_u4xMsaaQuality - 1;
    }
    // No MSAA
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

	DXHR(m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pMainDepthStencil->m_pTexture));
	DXHR(m_pD3dDevice->CreateDepthStencilView(m_pMainDepthStencil->m_pTexture, 0, &m_pMainDepthStencil->m_pDSV));

    // Bind the render target view and depth/stencil view to the pipeline.
	m_pD3dImmediateContext->OMSetRenderTargets(1, &m_pMainRenderTarget->m_pRTV, m_pMainDepthStencil->m_pDSV);

    // Set the viewport transform.

    m_sScreenViewport.TopLeftX = 0;
    m_sScreenViewport.TopLeftY = 0;
    m_sScreenViewport.Width = static_cast<float>(m_iClientWidth);
    m_sScreenViewport.Height = static_cast<float>(m_iClientHeight);
    m_sScreenViewport.MinDepth = 0.0f;
    m_sScreenViewport.MaxDepth = 1.0f;

    m_pD3dImmediateContext->RSSetViewports(1, &m_sScreenViewport);
}


ID3D11DepthStencilView* NDX11::GetDepthStencilView() 
{ 
	return m_pMainDepthStencil->m_pDSV; 
}

ID3D11RenderTargetView* NDX11::GetRenderTargetView() 
{ 
	return m_pMainRenderTarget->m_pRTV; 
}

NDX11VertexShader NDX11::CompileAndCreateVertexShader(N_G_COMPILE_SHADER_INFO &compileInfo, N_G_SHADER_LAYOUT &layout)
{
	CompileHLSLShaderFromMemory(compileInfo);
	return CreateVertexShader(compileInfo.BlobOut->GetBufferPointer(), compileInfo.BlobOut->GetBufferSize(), layout);
}

NDX11PixelShader NDX11::CompileAndCreatePixelShader(N_G_COMPILE_SHADER_INFO &compileInfo)
{
	CompileHLSLShaderFromMemory(compileInfo);
	return CreatePixelShader(compileInfo.BlobOut->GetBufferPointer(), compileInfo.BlobOut->GetBufferSize());
}

NDX11GeometryShader NDX11::CompileAndCreateGeometryShader(N_G_COMPILE_SHADER_INFO &compileInfo)
{
	CompileHLSLShaderFromMemory(compileInfo);
	return CreateGeometryShader(compileInfo.BlobOut->GetBufferPointer(), compileInfo.BlobOut->GetBufferSize());
}

NDX11ComputeShader NDX11::CompileAndCreateComputeShader(N_G_COMPILE_SHADER_INFO &compileInfo)
{
	CompileHLSLShaderFromMemory(compileInfo);
	return CreateComputeShader(compileInfo.BlobOut->GetBufferPointer(), compileInfo.BlobOut->GetBufferSize());
}

NDX11VertexShader NDX11::CreateVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	CreateHLSLVertexShader(shaderByteCode, byteCodeLength, nullptr, &vertexShader);
	return NDX11VertexShader(vertexShader, NULL);
}

NDX11VertexShader NDX11::CreateVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength, N_G_SHADER_LAYOUT &layout)
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	CreateHLSLVertexShader(shaderByteCode, byteCodeLength, nullptr, &vertexShader);
	CreateHLSLInputLayout(layout.layouts, layout.numElements, shaderByteCode, byteCodeLength, &inputLayout);
	return NDX11VertexShader(vertexShader, inputLayout);
}

NDX11PixelShader NDX11::CreatePixelShader(const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11PixelShader* pixelShader = nullptr;
	CreateHLSLPixelShader(shaderByteCode, byteCodeLength, nullptr, &pixelShader);
	return NDX11PixelShader(pixelShader);
}

NDX11GeometryShader NDX11::CreateGeometryShader(const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11GeometryShader* geometryShader = nullptr;
	CreateHLSLGeometryShader(shaderByteCode, byteCodeLength, nullptr, &geometryShader);
	return NDX11GeometryShader(geometryShader);
}

NDX11ComputeShader NDX11::CreateComputeShader(const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11ComputeShader* computeShader = nullptr;
	CreateHLSLComputeShader(shaderByteCode, byteCodeLength, nullptr, &computeShader);
	return NDX11ComputeShader(computeShader);
}

void NDX11::SetVertexShaderWLayout(NDX11VertexShader* shader)
{
	N_ASSERT(m_pD3dImmediateContext);
	if (shader)
	{
		m_pD3dImmediateContext->IASetInputLayout(shader->m_pInputLayout);
		m_pD3dImmediateContext->VSSetShader(shader->m_pVertexShader, NULL, 0);
	}
	else
	{
		m_pD3dImmediateContext->IASetInputLayout(nullptr);
		m_pD3dImmediateContext->VSSetShader(nullptr, NULL, 0);
	}
}

void NDX11::SetPixelShader(NDX11PixelShader* shader)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->PSSetShader((shader)?shader->m_pPixelShader:nullptr, NULL, 0);
}

void NDX11::SetGeometryShader(NDX11GeometryShader* shader)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->GSSetShader((shader) ? shader->m_pGeometryShader : nullptr, NULL, 0);
}

void NDX11::SetComputeShader(NDX11ComputeShader* shader)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->CSSetShader((shader) ? shader->m_pComputeShader : nullptr, NULL, 0);
}

void NDX11::CreateVertexBuffer(NDX11VertexBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
	, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount, UINT32 stride, UINT32 offset)
{
	CreateBuffer(&buffer.m_pBuffer, usage, structSize, D3D11_BIND_VERTEX_BUFFER, cpuAccFlag, miscFlag, initData, dataCount);
	buffer.m_iStride = stride;
	buffer.m_iOffset = offset;
}

void NDX11::CreateIndexBuffer(NDX11IndexBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
	, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount, N_RESOURCE_FORMAT format, UINT32 offset)
{
	CreateBuffer(&buffer.m_pBuffer, usage, structSize, D3D11_BIND_INDEX_BUFFER, cpuAccFlag, miscFlag, initData, dataCount);
	buffer.m_eFormat = format;
	buffer.m_iOffset = offset;
}

void NDX11::CreateConstantBuffer(NDX11ConstantBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize, UINT32 dataCount
	, UINT32 cpuAccFlag, UINT32 miscFlag)
{
	CreateBuffer(&buffer.m_pBuffer, usage, structSize, D3D11_BIND_CONSTANT_BUFFER, cpuAccFlag, miscFlag, NULL, dataCount);
}

void NDX11::CreateStructureBuffer(NDX11StructureBuffer& buffer, N_BUFFER_USAGE usage, UINT32 structSize
	, UINT32 bindFlag, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount)
{
	CreateBuffer(&buffer.m_pBuffer, usage, structSize, bindFlag, cpuAccFlag, miscFlag, initData, dataCount);
}

void* NDX11::MapConstantBuffer(NDX11ConstantBuffer& buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	DXHR(m_pD3dImmediateContext->Map(buffer.m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	void* cbData = (void*)MappedResource.pData;
	return cbData;
}

void NDX11::UnMapConstantBuffer(NDX11ConstantBuffer& buffer)
{
	m_pD3dImmediateContext->Unmap(buffer.m_pBuffer, 0);
}

void NDX11::SetConstantBufferData(NDX11ConstantBuffer& buffer, void* value, SIZE_T size)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(buffer.GetBuffer());

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	DXHR(m_pD3dImmediateContext->Map(buffer.GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	memcpy(mappedResource.pData, value, size);

	m_pD3dImmediateContext->Unmap(buffer.GetBuffer(), 0);

	//m_pD3dImmediateContext->UpdateSubresource(buffer.GetBuffer(),0, 0, value, 0, 0);
}

void NDX11::SetVSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->VSSetConstantBuffers(startSlot, numBuffers, (buffer) ? &buffer->m_pBuffer : 0);
}

void NDX11::SetPSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->PSSetConstantBuffers(startSlot, numBuffers, (buffer) ? &buffer->m_pBuffer : 0);
}

void NDX11::SetGSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->GSSetConstantBuffers(startSlot, numBuffers, (buffer) ? &buffer->m_pBuffer : 0);
}

void NDX11::SetCSConstantBuffers(UINT32 startSlot, UINT32 numBuffers, NDX11ConstantBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->CSSetConstantBuffers(startSlot, numBuffers, (buffer) ? &buffer->m_pBuffer : 0);
}

void NDX11::SetVSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (textures && textures[i]) ? textures[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->VSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetPSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (textures && textures[i]) ? textures[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->PSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetGSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (textures && textures[i]) ? textures[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->GSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetCSTextureResources(UINT32 startSlot, UINT32 numResources, NDX11Texture2DRes** textures)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (textures && textures[i]) ? textures[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->CSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetVSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (buffers && buffers[i]) ? buffers[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->VSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetPSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (buffers && buffers[i]) ? buffers[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->PSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetGSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (buffers && buffers[i]) ? buffers[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->GSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetCSStructureResources(UINT32 startSlot, UINT32 numResources, NDX11StructureBuffer** buffers)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numResources < N_G_MAX_TEXTURERESOURCEVIEWS);
	ID3D11ShaderResourceView* collection[N_G_MAX_TEXTURERESOURCEVIEWS];
	for (UINT32 i = 0; i < numResources; i++)
		collection[i] = (buffers && buffers[i]) ? buffers[i]->GetSRV() : NULL;
	m_pD3dImmediateContext->CSSetShaderResources(startSlot, numResources, collection);
}

void NDX11::SetCSUnoderedAccessViews(UINT32 startSlot, UINT32 numViews, NDX11Texture2DRes** textures)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numViews < N_G_MAX_UNORDEREDRESOURCEVIEWS);
	ID3D11UnorderedAccessView* collection[N_G_MAX_UNORDEREDRESOURCEVIEWS];
	for (UINT32 i = 0; i < numViews; i++)
		collection[i] = (textures && textures[i]) ? textures[i]->GetUAV() : NULL;
	m_pD3dImmediateContext->CSSetUnorderedAccessViews(startSlot, numViews, collection, (UINT*)(&collection));
}

void NDX11::SetCSUnoderedAccessViews(UINT32 startSlot, UINT32 numViews, NDX11StructureBuffer** buffers)
{
	N_ASSERT(m_pD3dImmediateContext);
	N_ASSERT(numViews < N_G_MAX_UNORDEREDRESOURCEVIEWS);
	ID3D11UnorderedAccessView* collection[N_G_MAX_UNORDEREDRESOURCEVIEWS];
	for (UINT32 i = 0; i < numViews; i++)
		collection[i] = (buffers && buffers[i]) ? buffers[i]->GetUAV() : NULL;
	m_pD3dImmediateContext->CSSetUnorderedAccessViews(startSlot, numViews, collection, (UINT*)(&collection));
}

void NDX11::BindVertexBuffer(UINT32 startSlot, UINT32 numBuffers, NDX11VertexBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->IASetVertexBuffers(startSlot, numBuffers, (buffer) ? &buffer->m_pBuffer : NULL
		, (buffer) ? &buffer->m_iStride : 0
		, (buffer) ? &buffer->m_iOffset : 0);
}

void NDX11::BindIndexBuffer(NDX11IndexBuffer* buffer)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->IASetIndexBuffer((buffer) ? buffer->m_pBuffer : NULL
		, (buffer) ? (DXGI_FORMAT)buffer->m_eFormat : (DXGI_FORMAT)0, (buffer) ? buffer->m_iOffset : 0);
}

void NDX11::SetPrimitiveTopology(UINT32 topology)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)topology);
}

void NDX11::DrawIndexed(UINT32 indexCount, UINT32 indexOffset, UINT32 vertexOffset)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void NDX11::Draw(UINT32 vertexCount, UINT32 vertexOffset)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->Draw(vertexCount, vertexOffset);
}

void NDX11::Dispatch(UINT32 threadGroupCountX, UINT32 threadGroupCountY, UINT32 threadGroupCountZ)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void NDX11::CreateTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(D3DX11CreateShaderResourceViewFromMemory(m_pD3dDevice, raw, size, 0, 0, &texture.m_pSRV, 0));
}

void NDX11::CreateTextureFromDesc(NDX11Texture2DRes& texture, N_G_TEXTURE2D_DESC &desc, UINTPTR initData, UINT32 dataSize)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	if (initData && dataSize > 0)
	{
		D3D11_SUBRESOURCE_DATA initSRData = { 0 };
		initSRData.SysMemPitch = dataSize;
		initSRData.pSysMem = (void *)initData;
		DXHR(m_pD3dDevice->CreateTexture2D(&desc, &initSRData, &texture.m_pTexture));
	}
	else
	{
		DXHR(m_pD3dDevice->CreateTexture2D(&desc, 0, &texture.m_pTexture));
	}
}

void NDX11::CreateDSSTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(DirectX::CreateDDSTextureFromMemory(m_pD3dDevice, raw, size, nullptr, &texture.m_pSRV));
}

void NDX11::WriteTextureToDDS(NDX11Texture2DRes& texture, std::string filepath, N_RESOURCE_FORMAT format)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DirectX::ScratchImage image;
	DirectX::ScratchImage bcImage;
	std::wstring wfilepath = s2ws(filepath);
	DXHR(DirectX::CaptureTexture(m_pD3dDevice, m_pD3dImmediateContext, texture.GetTexture(), image));
	if (format != 0)
	{
		DXHR(DirectX::Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), (DXGI_FORMAT)format, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, bcImage));
		//DXHR(DirectX::Compress(m_pD3dDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (DXGI_FORMAT)format, DirectX::TEX_COMPRESS_PARALLEL, 0.5f, bcImage));
		DXHR(DirectX::SaveToDDSFile(bcImage.GetImages(), bcImage.GetImageCount(), bcImage.GetMetadata(), DirectX::DDS_FLAGS_NONE, wfilepath.c_str()));
	}
	else
	{
		DXHR(DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, wfilepath.c_str()));
	}
	
}

void NDX11::CreateWICTextureFromMemory(NDX11Texture2DRes& texture, UCHAR* raw, UINT32 size)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DirectX::ScratchImage image;
	DirectX::TexMetadata metadata;
	DXHR(DirectX::LoadFromWICMemory(raw, size, DirectX::WIC_FLAGS_NONE, &metadata, image));
	DXHR(DirectX::CreateShaderResourceView(m_pD3dDevice,
		image.GetImages(), image.GetImageCount(),
		metadata, &texture.m_pSRV));
	//DXHR(DirectX::CreateWICTextureFromMemory(m_pD3dDevice, raw, size, nullptr, &texture.m_pSRV));
}

void NDX11::CreateDSV(NDX11Texture2DRes& texture)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	m_pD3dDevice->CreateDepthStencilView(texture.m_pTexture, 0, &texture.m_pDSV);
}

void NDX11::CreateDSV(NDX11Texture2DRes& texture, N_G_DEPTH_STENCIL_VIEW_DESC &desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateDepthStencilView(texture.m_pTexture, &desc, &texture.m_pDSV));
}

void NDX11::ClearDSV(NDX11Texture2DRes& texture, UINT32 clearFlags, float depth, UINT8 stencil)
{
	Clear((N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER) & clearFlags, nullptr, texture.m_pDSV, depth, stencil);
}


void NDX11::CreateSRV(NDX11Texture2DRes& texture)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateShaderResourceView(texture.m_pTexture, 0, &texture.m_pSRV));
}

void NDX11::CreateSRV(NDX11Texture2DRes& texture, N_G_SHADER_RESOURCE_VIEW_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateShaderResourceView(texture.m_pTexture, &desc, &texture.m_pSRV));
}

void NDX11::CreateSRV(NDX11StructureBuffer& buffer, N_G_SHADER_RESOURCE_VIEW_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateShaderResourceView(buffer.m_pBuffer, &desc, &buffer.m_pSRV));
}

void NDX11::CreateUAV(NDX11Texture2DRes& texture)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateUnorderedAccessView(texture.m_pTexture, nullptr, &texture.m_pUAV));
}

void NDX11::CreateUAV(NDX11Texture2DRes& texture, N_G_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateUnorderedAccessView(texture.m_pTexture, &desc, &texture.m_pUAV));
}

void NDX11::CreateUAV(NDX11StructureBuffer& buffer, N_G_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateUnorderedAccessView(buffer.m_pBuffer, &desc, &buffer.m_pUAV));
}

void NDX11::CreateRTV(NDX11Texture2DRes& texture)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateRenderTargetView(texture.m_pTexture, 0, &texture.m_pRTV));
}

void NDX11::CreateRTVFromDesc(NDX11Texture2DRes& texture, N_G_RENDER_TARGET_VIEW_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateRenderTargetView(texture.m_pTexture, &desc, &texture.m_pRTV));
}

void NDX11::ClearRTV(NDX11Texture2DRes& texture, float color[4])
{
	Clear(N_B_COLOR_BUFFER, texture.m_pRTV, nullptr, -1.f, 0, color[0], color[1], color[2], color[3]);
}

void NDX11::SetRenderTargets(NDX11Texture2DRes** rtvs, UINT32 rtvsNum, NDX11Texture2DRes* dsv)
{
	N_ASSERT(m_pD3dImmediateContext);
	std::vector<ID3D11RenderTargetView*> renderTargets;
	renderTargets.reserve(rtvsNum);
	for (UINT32 i = 0; i < rtvsNum; i++)
		renderTargets.push_back((rtvs && rtvs[i])?rtvs[i]->GetRTV():NULL);
	m_pD3dImmediateContext->OMSetRenderTargets(rtvsNum, renderTargets.data(), (dsv)?dsv->GetDSV():NULL);
}

void NDX11::CreateDepthStencilState(NDX11DepthStencilState& state, N_G_DEPTHSTENCIL_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateDepthStencilState(&desc, &state.m_pState));
}

void NDX11::CreateRasterizerState(NDX11RasterizerState& state, N_G_RASTERIZER_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateRasterizerState(&desc, &state.m_pState));
}

void NDX11::CreateBlendStateState(NDX11BlendState& state, N_G_BLEND_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateBlendState(&desc, &state.m_pState));
}

void NDX11::CreateSamplersState(NDX11SamplersState& state, N_G_SAMPLER_DESC& desc)
{
	N_ASSERT(m_pD3dDevice);
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	ID3D11SamplerState* createState = NULL;
	DXHR(m_pD3dDevice->CreateSamplerState(&desc, &createState));
	state.m_pState.push_back(createState);
}

void NDX11::SetDepthStencilState(NDX11DepthStencilState* state, UINT32 stencilRef)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->OMSetDepthStencilState((state) ? state->m_pState : NULL, stencilRef);
}

void NDX11::SetRasterizerState(NDX11RasterizerState* state)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->RSSetState((state) ? state->m_pState : NULL);
}

void NDX11::SetBlendStateState(NDX11BlendState* state, float blendFactors[4], UINT32 sampleMask)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->OMSetBlendState((state)?state->m_pState:NULL, blendFactors, sampleMask);
}

void NDX11::SetPSSamplersState(NDX11SamplersState* state, UINT32 startSlot, UINT32 numSamplers)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->PSSetSamplers(startSlot, numSamplers, (state)?&state->m_pState[0]:NULL);
}

void NDX11::SetCSSamplersState(NDX11SamplersState* state, UINT32 startSlot, UINT32 numSamplers)
{
	N_ASSERT(m_pD3dImmediateContext);
	m_pD3dImmediateContext->CSSetSamplers(startSlot, numSamplers, (state) ? &state->m_pState[0] : NULL);
}


void NDX11::CompileHLSLShaderFromMemory(N_G_COMPILE_SHADER_INFO &compileInfo)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBlob = NULL;
	hr = D3DCompile(compileInfo.RawBuffer, compileInfo.RawSize, compileInfo.ShaderName.c_str(), NULL, NULL, compileInfo.EntryPoint, compileInfo.ShaderModel,
		dwShaderFlags, 0, &(compileInfo.BlobOut), &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return;
	}
	if (pErrorBlob) pErrorBlob->Release();

	DXHR(hr);
}

void NDX11::CreateHLSLVertexShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11VertexShader** result)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateVertexShader(shaderByteCode, byteCodeLength, classLinkage, result));
}

void NDX11::CreateHLSLPixelShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11PixelShader** result)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreatePixelShader(shaderByteCode, byteCodeLength, classLinkage, result));
}

void NDX11::CreateHLSLGeometryShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11GeometryShader** result)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateGeometryShader(shaderByteCode, byteCodeLength, classLinkage, result));
}

void NDX11::CreateHLSLComputeShader(const void* shaderByteCode, SIZE_T byteCodeLength, ID3D11ClassLinkage* classLinkage, ID3D11ComputeShader** result)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateComputeShader(shaderByteCode, byteCodeLength, classLinkage, result));
}

void NDX11::CreateHLSLInputLayout(D3D11_INPUT_ELEMENT_DESC layout[], UINT32 numElements, const void* byteCode, SIZE_T byteCodeLength, ID3D11InputLayout** result)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);
	DXHR(m_pD3dDevice->CreateInputLayout(layout, numElements, byteCode, byteCodeLength, result));
}

void NDX11::CreateBuffer(ID3D11Buffer** buffer, N_BUFFER_USAGE usage, UINT32 structSize, UINT32 bingFlag
	, UINT32 cpuAccFlag, UINT32 miscFlag, void* initData, UINT32 dataCount)
{
	NScopedCriticalSection deviveThreadSafe(m_csDeviceLock);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = (D3D11_USAGE)usage;
	bd.ByteWidth = structSize * dataCount;
	bd.BindFlags = bingFlag;
	bd.CPUAccessFlags = cpuAccFlag;
	bd.MiscFlags = miscFlag;
	bd.StructureByteStride = structSize;

	if (initData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = initData;

		DXHR(m_pD3dDevice->CreateBuffer(&bd, &InitData, buffer));
	}
	else
	{
		DXHR(m_pD3dDevice->CreateBuffer(&bd, NULL, buffer));
	}
}