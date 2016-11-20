#pragma once
#include "../Macro/Macro.h"
#include "../Resource/NAssetProxy.h"
#include "../Resource/NAssetCache.h"
#include "../Resource/NAssetLoader.h"
#include "../Resource/NAssetLoadResult.h"
#include "../Resource/NAssetPool.h"
#include "NTexture2DRes.h"
#include "NDX11.h"
#include "../Thread/NCriticalSection.h"

class NAssetGatherer;
class NTexture2DPool;
class NDDSTextureLoader;

class NTexture2DProxy : public INAssetProxy
{
	friend NTexture2DPool;
	friend NDDSTextureLoader;
public:
	NTexture2DProxy(std::string key, UINT32 type = 0) : m_sKey(key), m_u32Type(type), m_pTexture(NULL), m_bIsInRemoveQueue(false), m_bIsSharedTexture(false){}
	virtual ~NTexture2DProxy() override { if (!m_bIsSharedTexture) { N_DELETE(m_pTexture); } }
	virtual std::string GetKey() override { return m_sKey; }
	inline NTexture2DRes* GetTexture() { return m_pTexture; }
	inline BOOL IsInRemoveQueue() { return m_bIsInRemoveQueue; }
	inline void SetSharedTexture(BOOL shared){ m_bIsSharedTexture = shared; }

protected:
	inline void SetTexture(NTexture2DRes* texture) { m_pTexture = texture; }
	inline UINT32 GetType() { return m_u32Type; }
	inline void SetInRemoveQueue(bool isInRemoveQueue = true) { m_bIsInRemoveQueue = isInRemoveQueue; }

	std::string m_sKey;
	UINT32 m_u32Type;
	NTexture2DRes* m_pTexture;
	BOOL m_bIsInRemoveQueue;
	BOOL m_bIsSharedTexture;
};

class NNativeTextureLoadResult : public INAssetLoadResult
{
public:
	NNativeTextureLoadResult(NTexture2DRes* texture) : m_pTexture(texture) {}
	inline NTexture2DRes* GetTexture() { return m_pTexture; }
protected:
	NTexture2DRes* m_pTexture;
};

class NDDSTextureLoader : public INAssetLoader
{
public:
	NDDSTextureLoader(NGraphicsDevice* dev);
	virtual ~NDDSTextureLoader();
	virtual INAssetLoadResult* LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy) override;
	virtual void ClearResult(INAssetLoadResult* result) override;

protected:
	NGraphicsDevice* m_pDevice;
};

class NWICTextureLoader : public INAssetLoader
{
public:
	NWICTextureLoader(NGraphicsDevice* dev);
	virtual ~NWICTextureLoader();
	virtual INAssetLoadResult* LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy) override;
	virtual void ClearResult(INAssetLoadResult* result) override;

protected:
	NGraphicsDevice* m_pDevice;
};

class NTexture2DPool : public INAssetPool
{
public:
	enum TEXTURE2D_TYPE
	{
		TEXTURE2D_TYPE_FLOAT = 0,
		TEXTURE2D_TYPE_UINT = 1,
		TEXTURE2D_TYPE_N = 2
	};

	NTexture2DPool(NAssetGatherer* gatherer);
	virtual ~NTexture2DPool();

	virtual void AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result) override;

	void Init(NGraphicsDevice* dev);
	NTexture2DProxy* GetTexture(std::string key, UINT32 type = 0, BOOL async = true, NTexture2DRes* asset = 0);
	BOOL RemoveTexture(INAssetProxy* proxy, BOOL async = true);

protected:
	NDDSTextureLoader* m_pDDSLoader;
	NWICTextureLoader* m_pWICLoader;
	NFileAssetCache<std::string, NTexture2DProxy*>* m_pCache;
	NTexture2DRes* m_pDefTextures[TEXTURE2D_TYPE_N];

	NAssetGatherer* m_pGatherer;

	NCriticalSection m_csPoolLock;
};