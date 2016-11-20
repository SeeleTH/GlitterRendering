#pragma once
#include "../Macro/Macro.h"
#include <map>

template<class Key, class Type>
class INAssetCache
{
public:
	virtual ~INAssetCache() {}
	virtual void AddAsset(Key key,Type asset) = 0;
	virtual Type GetAsset(Key key) = 0;
	virtual BOOL IsExist(Key key) = 0;
	virtual Type PopAsset(Key key) = 0;
	virtual void FreeAssets() = 0;
	virtual void ClearAssets() = 0;
};

template<class Key, class Type>
class NFileAssetCache : public INAssetCache<Key, Type>
{
public:
	NFileAssetCache() { }

	virtual ~NFileAssetCache() override 
	{ 
		FreeAssets(); 
	}

	virtual void AddAsset(Key key, Type asset) override
	{
		m_mapCache.insert(std::pair<Key, Type>(key, asset));
	}

	virtual Type GetAsset(Key key) override
	{
		return m_mapCache[key];
	}

	virtual BOOL IsExist(Key key) override
	{
		return m_mapCache.count(key) > 0;
	}

	virtual Type PopAsset(Key key) override
	{
		Type value = m_mapCache[key];
		m_mapCache.erase(key);
		return value;
	}

	virtual void FreeAssets() override
	{
		for (auto it = m_mapCache.begin(); it != m_mapCache.end(); it++)
		{
			N_DELETE((*it).second);
		}
		ClearAssets();
	}

	virtual void ClearAssets() override
	{
		m_mapCache.clear();
	}

protected:
	std::map<Key, Type> m_mapCache;
};