#include "NAssetGatherer.h"

#include <Windows.h>
#include <locale>
#include <codecvt>

NFileAssetStreamer NAssetGatherer::m_fileStreamer;

NAssetGatherer::NAssetGatherer()
	: m_u32ProcessingAssetN(0)
	, m_bIsStopThread(false)
{
}

NAssetGatherer::~NAssetGatherer()
{
	if (!GetSafeStopThreadFlag())
	{
		while (m_u32ProcessingAssetN > 0)
		{
			GatherResult processResult;
			if (m_qResultProxies.try_pop(processResult))
			{
				m_u32ProcessingAssetN--;
				N_DELETE(processResult.result);
			}
		}
	}
	Exit(0);
}


NAssetGatherer::GATHER_REQ_STATE NAssetGatherer::AddRequest(INAssetProxy* proxy
	, INAssetLoader* loader, INAssetPool* pool)
{
	m_u32ProcessingAssetN++;
	GatherInstruct inst;
	inst.proxy = proxy;
	inst.pool = pool;
	inst.loader = loader;
	m_qProcessProxies.push(inst);
	if (!GetSafeStopThreadFlag())
		Run();
	return GATHER_REQ_ACCEPTED;
}

void NAssetGatherer::InstantLoadAsset(INAssetProxy* proxy
	, INAssetLoader* loader, INAssetPool* pool)
{
	INAssetLoadResult* result = loader->LoadProxy_GatherThread(&m_fileStreamer, proxy);
	pool->AddLoadedProxy(proxy, result);
	loader->ClearResult(result);
}

void NAssetGatherer::SafeStopThread(BOOL stop)
{
	NScopedCriticalSection stopFlagLock(m_csStopThread);
	m_bIsStopThread = stop;
}


void NAssetGatherer::OnUpdate()
{
	ReturnResults();
}


void NAssetGatherer::VThreadProc(void)
{
	while (true)
	{
		GatherInstruct inst;
		while (m_qProcessProxies.try_pop(inst))
		{
			INAssetLoadResult* result = inst.loader->LoadProxy_GatherThread(&m_fileStreamer, inst.proxy);

			GatherResult queueResult;
			queueResult.inst = inst;
			queueResult.result = result;
			m_qResultProxies.push(queueResult);

			if (GetSafeStopThreadFlag())
			{
				Stop();
			}
		}

		Stop();
	}
}


void NAssetGatherer::ReturnResults()
{
	GatherResult result;
	while (m_qResultProxies.try_pop(result))
	{
		result.inst.pool->AddLoadedProxy(result.inst.proxy, result.result);
		result.inst.loader->ClearResult(result.result);
		m_u32ProcessingAssetN--;
	}
}

BOOL NAssetGatherer::GetSafeStopThreadFlag()
{
	NScopedCriticalSection stopFlagLock(m_csStopThread);
	return m_bIsStopThread;
}
