#pragma once
#include "../../Macro/Macro.h"
#include "../../Resource/NResCache.h"
#include "../../Graphics/NGraphics.h"

class NVertexShaderNResLoader : public INResLoader
{
public:
	NVertexShaderNResLoader(NGraphicsDevice* dev) : m_pDevice(dev) {}

	virtual BOOL VUseRawFile() { return false; }
	virtual BOOL VDiscardRawBufferAfterLoad() { return true; }
	virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
	virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle);
	virtual std::string VGetPattern() { return "*_vs.cso"; }

protected:
	NGraphicsDevice* m_pDevice;
};

class NVertexShaderNResExtraData : public INResExtraData
{
public:
	NVertexShaderNResExtraData(NGraphicsDevice* dev, void* shaderByteCode, SIZE_T shaderByteCodeLength)
		: m_bIsInit(false) 
		, m_pDevice(dev)
		, m_vertexShader(NULL)
	{
		m_pShaderByteCode = N_NEW UCHAR[shaderByteCodeLength];
		m_sShaderByteCodeLength = shaderByteCodeLength;
		memcpy(m_pShaderByteCode, shaderByteCode, m_sShaderByteCodeLength);
	}

	virtual std::string VToString() { return "VertexShaderNResExtraData"; }

	inline NVertexShader* GetShader() { return &m_vertexShader; }
	inline const BOOL IsInitialized() { return m_bIsInit; }

	void Initialize(N_G_SHADER_LAYOUT& layout)
	{
		if (IsInitialized())
			return;
		m_vertexShader = m_pDevice->CreateVertexShader(m_pShaderByteCode, m_sShaderByteCodeLength, layout);
		N_DELETE_ARRAY(m_pShaderByteCode);
		m_sShaderByteCodeLength = 0;
		m_bIsInit = true;
	}

	void Initialize()
	{
		if (IsInitialized())
			return;
		m_vertexShader = m_pDevice->CreateVertexShader(m_pShaderByteCode, m_sShaderByteCodeLength);
		N_DELETE_ARRAY(m_pShaderByteCode);
		m_sShaderByteCodeLength = 0;
		m_bIsInit = true;
	}

protected:
	BOOL m_bIsInit;
	NVertexShader m_vertexShader;

	NGraphicsDevice* m_pDevice;
	void* m_pShaderByteCode;
	SIZE_T m_sShaderByteCodeLength;
};

class NPixelShaderNResLoader : public INResLoader
{
public:
	NPixelShaderNResLoader(NGraphicsDevice* dev) : m_pDevice(dev) {}

	virtual BOOL VUseRawFile() { return false; }
	virtual BOOL VDiscardRawBufferAfterLoad() { return true; }
	virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
	virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle);
	virtual std::string VGetPattern() { return "*_ps.cso"; }

protected:
	NGraphicsDevice* m_pDevice;
};

class NPixelShaderNResExtraData : public INResExtraData
{
public:
	NPixelShaderNResExtraData(NPixelShader pixelShader)
		: m_pixelShader(pixelShader)
	{}

	virtual std::string VToString() { return "PixelShaderNResExtraData"; }

	inline NPixelShader* GetShader() { return &m_pixelShader; }

protected:
	NPixelShader m_pixelShader;
};

class NGeometryShaderNResLoader : public INResLoader
{
public:
	NGeometryShaderNResLoader(NGraphicsDevice* dev) : m_pDevice(dev) {}

	virtual BOOL VUseRawFile() { return false; }
	virtual BOOL VDiscardRawBufferAfterLoad() { return true; }
	virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
	virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle);
	virtual std::string VGetPattern() { return "*_gs.cso"; }

protected:
	NGraphicsDevice* m_pDevice;
};

class NGeometryShaderNResExtraData : public INResExtraData
{
public:
	NGeometryShaderNResExtraData(NGeometryShader geometryShader)
		: m_geometryShader(geometryShader)
	{}

	virtual std::string VToString() { return "GeometryShaderNResExtraData"; }

	inline NGeometryShader* GetShader() { return &m_geometryShader; }

protected:
	NGeometryShader m_geometryShader;
};

class NComputeShaderNResLoader : public INResLoader
{
public:
	NComputeShaderNResLoader(NGraphicsDevice* dev) : m_pDevice(dev) {}

	virtual BOOL VUseRawFile() { return false; }
	virtual BOOL VDiscardRawBufferAfterLoad() { return true; }
	virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
	virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle);
	virtual std::string VGetPattern() { return "*_cs.cso"; }

protected:
	NGraphicsDevice* m_pDevice;
};

class NComputeShaderNResExtraData : public INResExtraData
{
public:
	NComputeShaderNResExtraData(NComputeShader computeShader)
		: m_computeShader(computeShader)
	{}

	virtual std::string VToString() { return "ComputeShaderNResExtraData"; }

	inline NComputeShader* GetShader() { return &m_computeShader; }

protected:
	NComputeShader m_computeShader;
};