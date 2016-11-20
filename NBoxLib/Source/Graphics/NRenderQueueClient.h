#pragma once

#include "../Macro/Macro.h"
#include "NGraphics.h"

class NRenderQueueClient
{
public:
    virtual void VRender(UINTPTR userData, UINT32 renderType, NGraphicsDevice* dev) = 0;
};