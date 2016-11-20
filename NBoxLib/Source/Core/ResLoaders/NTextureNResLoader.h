#pragma once
#include "../../Macro/Macro.h"
#include "../../Resource/NResCache.h"
#include "../../Graphics/NGraphics.h"

class NDDSTextureNResLoader : public INResLoader
{
public:
	NDDSTextureNResLoader(NGraphicsDevice* dev) : m_pDevice(dev) {}

	virtual BOOL VUseRawFile() { return false; }
	virtual BOOL VDiscardRawBufferAfterLoad() { return true; }
	virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
	virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle);
	virtual std::string VGetPattern() { return "*.dds"; }

protected:
	NGraphicsDevice* m_pDevice;
};

class NDDSTextureNResExtraData : public INResExtraData
{
public:
	NDDSTextureNResExtraData(NTexture2DRes* texture) : m_pTexture(texture) {}
	virtual std::string VToString() { return "DDSTextureNResExtraData"; }

	inline NTexture2DRes* GetTexture() { return m_pTexture; }

protected:
	NTexture2DRes* m_pTexture;
};