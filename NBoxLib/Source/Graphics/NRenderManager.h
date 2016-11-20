#pragma once

#include "../Macro/Macro.h"
#include "NGraphics.h"
#include "NRenderableObject.h"
#include "NRenderQueue.h"

#include <vector>
#include <memory>

class NRenderManagerBase
{
public:
    NRenderManagerBase(){}
    virtual ~NRenderManagerBase(){}

    // Add render tasks to render queue(s) by querying visible objects
    virtual void VInit() = 0;
    virtual void VRender() = 0;
    virtual void VDestroy() = 0;

    virtual void VRegisterObject(NRenderableObject* object) = 0;
    virtual void VUnregisterObject(NRenderableObject* object) = 0;
    virtual void VClearObjects() = 0;
};

class NRenderManager : public NRenderManagerBase
{
public:
    NRenderManager(NGraphicsDevice* dev);
    virtual ~NRenderManager();

    virtual void VInit();
    virtual void VRender();
    virtual void VDestroy();

    virtual void VRegisterObject(NRenderableObject* object);
    virtual void VUnregisterObject(NRenderableObject* object);
    virtual void VClearObjects();

protected:
    NGraphicsDevice* m_pDevice;

    typedef std::vector<NRenderableObject*> NRenderableObjects;
    NRenderableObjects m_vRenderableObjects;

    NRenderQueue* m_pRenderQueue;
};