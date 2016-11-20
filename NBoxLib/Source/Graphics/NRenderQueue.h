#pragma once

#include "../Macro/Macro.h"
#include "NGraphics.h"
#include "NRenderCmd.h"
#include <vector>

class NRenderQueue
{
protected:

public:
    NRenderQueue(NGraphicsDevice* dev);
    virtual ~NRenderQueue();

	virtual void VInit();
	virtual void VDestroy();
	virtual void VRender();

	void QueueCommandList(NRenderCmdList& cmdList);
	void FlushCommandList();

protected:
    void SwitchDoubleQueue();
	NRenderCmdList* GetIdleQueue(){ return &m_pRenderQueueEntry[!m_iCurProcQueueEntry]; }
	NRenderCmdList* GetActiveQueue(){ return &m_pRenderQueueEntry[m_iCurProcQueueEntry]; }

	NRenderCmdList m_pRenderQueueEntry[2];
    UINT32 m_iCurProcQueueEntry;

    NGraphicsDevice* m_pDevice;
};