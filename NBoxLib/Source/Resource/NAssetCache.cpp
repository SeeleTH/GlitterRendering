#include "NAssetCache.h"

//template<class Key, class Type>
//NFileAssetCache<Key, Type>::NFileAssetCache()
//{
//
//}
//
//template<class Key, class Type>
//NFileAssetCache<Key, Type>::~NFileAssetCache()
//{
//	FreeAssets();
//}
//
//template<class Key, class Type>
//void NFileAssetCache<Key, Type>::AddAsset(Key key, Type asset)
//{
//	m_mapCache.insert(std::pair<Key, Type>(key, asset));
//}
//
//template<class Key, class Type>
//Type NFileAssetCache<Key, Type>::GetAsset(Key key)
//{
//	return m_mapCache[key];
//}
//
//template<class Key, class Type>
//BOOL NFileAssetCache<Key, Type>::IsExist(Key key)
//{
//	return m_mapCache.count(key) > 0;
//}
//
//template<class Key, class Type>
//Type NFileAssetCache<Key, Type>::PopAsset(Key key)
//{
//	Type value = m_mapCache[key];
//	m_mapCache.erase(key);
//}
//
//template<class Key, class Type>
//void NFileAssetCache<Key, Type>::FreeAssets()
//{
//	for (auto it = m_mapCache.begin(); it != m_mapCache.end(); it++)
//	{
//		N_DELETE(*it);
//	}
//	ClearAssets();
//}
//
//template<class Key, class Type>
//void NFileAssetCache<Key, Type>::ClearAssets()
//{
//	m_mapCache.clear();
//}
