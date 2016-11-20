#include "NRenderManager.h"

NRenderManager::NRenderManager(NGraphicsDevice* dev)
: m_pDevice(dev)
, m_pRenderQueue(0)
{

}

NRenderManager::~NRenderManager()
{
    VClearObjects();
}

void NRenderManager::VInit()
{
	m_pRenderQueue = new NRenderQueue(m_pDevice);
    m_pRenderQueue->VInit();
}

void NRenderManager::VRender()
{
	NRenderCmdList cmdList;
	// Clear cmd
	cmdList.CmdClear();

    // Add visible objects (all for now) to the render queue
    for (NRenderableObjects::iterator it = m_vRenderableObjects.begin(); it != m_vRenderableObjects.end(); it++)
    {
        if (*it)
        {
            //m_pRenderQueue->AddItem(0, NULL, (*it)->VGetRenderQueueClient(), (*it)->VGetRenderData(), (*it)->VGetDist(), (*it)->VGetAlpha());
        }
    }

	// Present
	cmdList.CmdPresent();

	m_pRenderQueue->QueueCommandList(cmdList);

    // Actually this should sent command to render queues thread..
    m_pRenderQueue->VRender();
}

void NRenderManager::VDestroy()
{
    m_pRenderQueue->VDestroy();
	N_DELETE(m_pRenderQueue);
}

void NRenderManager::VRegisterObject(NRenderableObject* object)
{
    NRenderableObjects::iterator findIt = std::find(m_vRenderableObjects.begin(), m_vRenderableObjects.end(), object);
    if (findIt == m_vRenderableObjects.end())
    {
        m_vRenderableObjects.push_back(object);
    }
}

void NRenderManager::VUnregisterObject(NRenderableObject* object)
{
    NRenderableObjects::iterator findIt = std::find(m_vRenderableObjects.begin(), m_vRenderableObjects.end(), object);
    if (findIt != m_vRenderableObjects.end())
    {
        m_vRenderableObjects.erase(findIt);
    }
}

void NRenderManager::VClearObjects()
{
    m_vRenderableObjects.clear();
}