#pragma once

#include "../Macro/Macro.h"
#include "NRenderQueue.h"

class NDX11RenderQueue : public NRenderQueue
{
public:
    NDX11RenderQueue(NGraphicsDevice* dev);
    virtual ~NDX11RenderQueue();

    virtual void VInit();
    virtual void VDestroy();
    virtual void VRender();

protected:
    void SortActiveQueue();
};