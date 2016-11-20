#include "NRenderQueue.h"


NRenderQueue::NRenderQueue(NGraphicsDevice* dev)
: m_pDevice(dev)
, m_iCurProcQueueEntry(0)
{
}

NRenderQueue::~NRenderQueue()
{

}

void NRenderQueue::VInit()
{
}

void NRenderQueue::VDestroy()
{
}

void NRenderQueue::VRender()
{
	SwitchDoubleQueue();
	
	GetActiveQueue()->ExecuteList(*m_pDevice);
}


void NRenderQueue::QueueCommandList(NRenderCmdList& cmdList)
{
	cmdList.MoveCommandsTo(*GetIdleQueue());
}

void NRenderQueue::FlushCommandList()
{
    GetIdleQueue()->Flush();
}

void NRenderQueue::SwitchDoubleQueue()
{
    m_iCurProcQueueEntry ^= 1;
	FlushCommandList();
}
