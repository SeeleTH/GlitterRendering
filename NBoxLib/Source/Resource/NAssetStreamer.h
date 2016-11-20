#pragma once
#include "../Macro/Macro.h"
#include "../Util/NStream.h"

class INAssetStreamer
{
public:
	virtual ~INAssetStreamer() {}
	virtual INStream* GetStream(std::string key) = 0;
};

class NFileAssetStreamer : public INAssetStreamer
{
public:
	virtual INStream* GetStream(std::string key) override;
};