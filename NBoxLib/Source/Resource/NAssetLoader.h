#pragma once
#include "../Macro/Macro.h"
#include "../Util/NStream.h"
#include "NAssetProxy.h"
#include "NAssetLoadResult.h"

class INAssetStreamer;

class INAssetLoader
{
public:
	virtual ~INAssetLoader() {}
	virtual INAssetLoadResult* LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy) = 0;
	virtual void ClearResult(INAssetLoadResult* result) = 0;
};