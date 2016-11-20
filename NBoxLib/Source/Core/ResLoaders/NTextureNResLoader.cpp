#include "NTextureNResLoader.h"

BOOL NDDSTextureNResLoader::VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle)
{
	if (m_pDevice == NULL)
		return false;

	NTexture2DRes* texture = new NTexture2DRes();
	m_pDevice->CreateDSSTextureFromMemory(*texture, (UCHAR*)rawBuffer, rawSize);

	std::shared_ptr<INResExtraData> extraData(new NDDSTextureNResExtraData(texture));
	handle->SetExtra(extraData);

	return true;
}