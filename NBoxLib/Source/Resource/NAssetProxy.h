#pragma once
#include "../Macro/Macro.h"
#include <string>

class INAssetProxy
{
public:
	virtual ~INAssetProxy() {}
	virtual std::string GetKey() = 0;
};