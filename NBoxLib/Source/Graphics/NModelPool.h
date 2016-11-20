#pragma once
#include "../Macro/Macro.h"
#include "../Resource/NAssetProxy.h"
#include "../Resource/NAssetCache.h"
#include "../Resource/NAssetLoader.h"
#include "../Resource/NAssetLoadResult.h"
#include "../Resource/NAssetPool.h"
#include "NDX11.h"

class NAssetGatherer;
class NTexture2DProxy;

struct ModelData
{

};

class NModelProxy : public INAssetProxy
{
public:

	NModelProxy(std::string key, UINT32 type = 0) : m_sKey(key), m_u32Type(type) {}
	virtual ~NModelProxy() override { }
	virtual std::string GetKey() override { return m_sKey; }

protected:
	std::string m_sKey;
	UINT32 m_u32Type;
};

class NModelLoadResult : public INAssetLoadResult
{
public:
	NModelLoadResult(ModelData* model) : m_pModelData(model) {}
	inline ModelData* GetModelData() { return m_pModelData; }
protected:
	ModelData* m_pModelData;
};

class NTexture2DPool;

class NOBJModelLoader : public INAssetLoader
{
public:
	NOBJModelLoader(NGraphicsDevice* dev);
	virtual ~NOBJModelLoader();
	virtual INAssetLoadResult* LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy) override;
	virtual void ClearResult(INAssetLoadResult* result) override;

protected:
	NGraphicsDevice* m_pDevice;
};

class NModelPool : public INAssetPool
{
public:
	enum MODEL_TYPE
	{
		MODEL_TYPE_OBJ = 0,
		MODEL_TYPE_FBX = 1
	};
	NModelPool(NGraphicsDevice* dev, NAssetGatherer* gatherer, NTexture2DPool* texturePool);
	virtual ~NModelPool();

	virtual void AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result) override;
	NModelProxy* GetModel(std::string key, UINT32 type = 0, BOOL async = true);

protected:
	NAssetGatherer* m_pGatherer;
	NTexture2DPool* m_pTexturePool;
};