#pragma once
#include "../Macro/Macro.h"
#include "NAssetProxy.h"
#include "NAssetLoadResult.h"

class INAssetPool
{
public:
	virtual ~INAssetPool() {}
	virtual void AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result) = 0;
};