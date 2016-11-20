#include "NShaderNResLoader.h"

BOOL NVertexShaderNResLoader::VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle)
{
	if (m_pDevice == NULL)
		return false;

	std::shared_ptr<INResExtraData> extraData(new NVertexShaderNResExtraData(m_pDevice, rawBuffer, rawSize));
	handle->SetExtra(extraData);

	return true;
}


BOOL NPixelShaderNResLoader::VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle)
{
	if (m_pDevice == NULL)
		return false;

	NPixelShader shader = m_pDevice->CreatePixelShader(rawBuffer, rawSize);
	std::shared_ptr<INResExtraData> extraData(new NPixelShaderNResExtraData(shader));
	handle->SetExtra(extraData);

	return true;
}

BOOL NGeometryShaderNResLoader::VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle)
{
	if (m_pDevice == NULL)
		return false;

	NGeometryShader shader = m_pDevice->CreateGeometryShader(rawBuffer, rawSize);
	std::shared_ptr<INResExtraData> extraData(new NGeometryShaderNResExtraData(shader));
	handle->SetExtra(extraData);

	return true;
}

BOOL NComputeShaderNResLoader::VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle)
{
	if (m_pDevice == NULL)
		return false;

	NComputeShader shader = m_pDevice->CreateComputeShader(rawBuffer, rawSize);
	std::shared_ptr<INResExtraData> extraData(new NComputeShaderNResExtraData(shader));
	handle->SetExtra(extraData);

	return true;
}