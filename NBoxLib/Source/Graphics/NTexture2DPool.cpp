#include "NTexture2DPool.h"
#include "../Resource/NAssetGatherer.h"


NDDSTextureLoader::NDDSTextureLoader(NGraphicsDevice* dev)
	: m_pDevice(dev)
{
}

NDDSTextureLoader::~NDDSTextureLoader()
{
}

INAssetLoadResult* NDDSTextureLoader::LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy)
{
	INStream* content = streamer->GetStream(proxy->GetKey());
	UINT64 size = content->ReadByte(0, 0);
	if (content->CheckErr() != INStream::NSTREAM_SUCESS || size <= 0)
	{
		N_ERROR("Cannot load file buffer failed: \nKey = " + proxy->GetKey());
		return new NNativeTextureLoadResult(nullptr);
	}
	char* buffer = new char[size];
	content->ReadByte(buffer, size);
	NTexture2DRes* result = new NTexture2DRes();
	if (content->CheckErr() != INStream::NSTREAM_ERROR)
	{
		m_pDevice->CreateDSSTextureFromMemory(*result, (UCHAR*)buffer, (UINT32)size);
	}
	else
	{
		N_ERROR("Cannot load DDS Texture: \nKey = " + proxy->GetKey());
	}
	N_DELETE(buffer);
	N_DELETE(content);

	if (!result->GetSRV())
	{
		N_DELETE(result);
	}

	return new NNativeTextureLoadResult(result);
}

void NDDSTextureLoader::ClearResult(INAssetLoadResult* result)
{
	NNativeTextureLoadResult* convResult = (NNativeTextureLoadResult*)result;
	N_DELETE(convResult);
}

NWICTextureLoader::NWICTextureLoader(NGraphicsDevice* dev)
	: m_pDevice(dev)
{
}

NWICTextureLoader::~NWICTextureLoader()
{
}

INAssetLoadResult* NWICTextureLoader::LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy)
{
	INStream* content = streamer->GetStream(proxy->GetKey());
	UINT64 size = content->ReadByte(0, 0);
	if (content->CheckErr() != INStream::NSTREAM_SUCESS || size <= 0)
	{
		N_ERROR("Cannot load file buffer failed: \nKey = " + proxy->GetKey());
	}
	char* buffer = new char[size];
	content->ReadByte(buffer, size);
	NTexture2DRes* result = new NTexture2DRes();
	if (content->CheckErr() != INStream::NSTREAM_ERROR)
	{
		CoInitialize(nullptr);
		m_pDevice->CreateWICTextureFromMemory(*result, (UCHAR*)buffer, (UINT32)size);
		CoUninitialize();
	}
	else
	{
		N_ERROR("Cannot load DDS Texture: \nKey = " + proxy->GetKey());
	}
	N_DELETE(buffer);
	N_DELETE(content);

	if (!result->GetSRV())
	{
		N_DELETE(result);
	}

	return new NNativeTextureLoadResult(result);
}

void NWICTextureLoader::ClearResult(INAssetLoadResult* result)
{
	NNativeTextureLoadResult* convResult = (NNativeTextureLoadResult*)result;
	N_DELETE(convResult);
}

NTexture2DPool::NTexture2DPool(NAssetGatherer* gatherer)
	: m_pDDSLoader(0)
	, m_pWICLoader(0)
	, m_pGatherer(gatherer)
	, m_pCache(0)
{
	for (UINT32 i = 0; i < TEXTURE2D_TYPE_N; i++)
		m_pDefTextures[i] = NULL;
}

NTexture2DPool::~NTexture2DPool()
{
	for (UINT32 i = 0; i < TEXTURE2D_TYPE_N; i++)
	{
		if (m_pCache->IsExist(""))
		{
			m_pCache->PopAsset("");
		}
		N_DELETE(m_pDefTextures[i]);
	}

	N_DELETE(m_pCache);
	N_DELETE(m_pDDSLoader);
	N_DELETE(m_pWICLoader);
}

void NTexture2DPool::AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result)
{
	NTexture2DProxy* texProxy = (NTexture2DProxy*)proxy;
	NNativeTextureLoadResult* texResult = (NNativeTextureLoadResult*)result;
	if (texProxy)
	{
		if (!texResult || !texResult->GetTexture())
		{
			texProxy->SetSharedTexture(true);
			texProxy->SetTexture(m_pDefTextures[texProxy->GetType()]);
		}
		else
		{
			texProxy->SetSharedTexture(false);
			texProxy->SetTexture(texResult->GetTexture());
		}
	}
}

void NTexture2DPool::Init(NGraphicsDevice* dev)
{
	m_pDDSLoader = new NDDSTextureLoader(dev);
	m_pWICLoader = new NWICTextureLoader(dev);
	m_pCache = new NFileAssetCache<std::string, NTexture2DProxy*>();

	{
		m_pDefTextures[TEXTURE2D_TYPE_FLOAT] = new NTexture2DRes();
		N_G_TEXTURE2D_DESC texDesc;
		texDesc.Width = 1;
		texDesc.Height = 1;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		UINT8 initData[4] = { 255, 255, 255, 255 };
		dev->CreateTextureFromDesc(*m_pDefTextures[TEXTURE2D_TYPE_FLOAT], texDesc, (UINTPTR)&initData, sizeof(UINT8) * 4);
		dev->CreateSRV(*m_pDefTextures[TEXTURE2D_TYPE_FLOAT]);
		m_pDefTextures[TEXTURE2D_TYPE_FLOAT]->ReleaseTexture();
	}

	{
		m_pDefTextures[TEXTURE2D_TYPE_UINT] = new NTexture2DRes();
		N_G_TEXTURE2D_DESC texDesc;
		texDesc.Width = 1;
		texDesc.Height = 1;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		UINT16 initData[4] = { 0, 0, 0, 0 };
		dev->CreateTextureFromDesc(*m_pDefTextures[TEXTURE2D_TYPE_UINT], texDesc, (UINTPTR)&initData, sizeof(UINT16) * 4);
		dev->CreateSRV(*m_pDefTextures[TEXTURE2D_TYPE_UINT]);
		m_pDefTextures[TEXTURE2D_TYPE_UINT]->ReleaseTexture();
	}

}

NTexture2DProxy* NTexture2DPool::GetTexture(std::string key, UINT32 type, BOOL async, NTexture2DRes* asset)
{
	NScopedCriticalSection csLock(m_csPoolLock);

	if (m_pCache->IsExist(key))
	{
		NTexture2DProxy* proxy = m_pCache->GetAsset(key);
		if (asset && proxy->GetTexture() != asset)
		{
			proxy->SetTexture(asset);
			proxy->SetSharedTexture(false);
		}
		return proxy;
	}
	else
	{
		NTexture2DProxy* proxy = new NTexture2DProxy(key);
		proxy->m_u32Type = type;
		m_pCache->AddAsset(key, proxy);
		if (asset)
		{
			proxy->SetTexture(asset);
			proxy->SetSharedTexture(false);
			return proxy;
		}
		else
		{
			proxy->SetTexture(m_pDefTextures[type]);
			proxy->SetSharedTexture(true);
			if (key != "")
			{
				N_ASSERT(m_pGatherer);
				BOOL isWIC = (proxy->GetKey().find(".dds") == std::string::npos);
				if (async)
				{
					if (isWIC)
						m_pGatherer->AddRequest(proxy, m_pWICLoader, this);
					else
						m_pGatherer->AddRequest(proxy, m_pDDSLoader, this);
				}
				else
				{
					if (isWIC)
						m_pGatherer->InstantLoadAsset(proxy, m_pWICLoader, this);
					else
						m_pGatherer->InstantLoadAsset(proxy, m_pDDSLoader, this);
				}
			}
			return proxy;
		}
	}
}

BOOL NTexture2DPool::RemoveTexture(INAssetProxy* proxy, BOOL async)
{
	NScopedCriticalSection csLock(m_csPoolLock);

	return false;
}