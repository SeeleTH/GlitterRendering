#pragma once
#include "../Macro/Macro.h"
#include "NAssetPool.h"
#include "NAssetLoader.h"
#include "../Thread/NConcurrentQueue.h"
#include "../Thread/NRunnable.h"
#include "../Thread/NCriticalSection.h"
#include "NAssetStreamer.h"
#include <string>

class NAssetGatherer : public NRunnable
{
public:
	enum GATHER_REQ_STATE
	{
		GATHER_REQ_ACCEPTED = 0,
		GATHER_REQ_REJECTED = 1,
		GATHER_REQ_ERROR = 2
	};

	NAssetGatherer();
	~NAssetGatherer();

	GATHER_REQ_STATE AddRequest(INAssetProxy* proxy, INAssetLoader* loader, INAssetPool* pool);
	void SafeStopThread(BOOL stop = true);

	void InstantLoadAsset(INAssetProxy* proxy, INAssetLoader* loader, INAssetPool* pool);

	void OnUpdate();

protected:
	struct GatherInstruct
	{
		INAssetProxy* proxy; // key and obj slot
		INAssetLoader* loader; // load raw into obj
		INAssetPool* pool; // callback when finish
	};

	struct GatherResult
	{
		GatherInstruct inst;
		INAssetLoadResult* result;
	};

	virtual void VThreadProc(void) override;

	void ReturnResults();
	BOOL GetSafeStopThreadFlag();

	NConcurrentQueue<GatherInstruct> m_qProcessProxies;
	NConcurrentQueue<GatherResult> m_qResultProxies;

	BOOL m_bIsStopThread;
	NCriticalSection m_csStopThread;

	UINT32 m_u32ProcessingAssetN;

	static NFileAssetStreamer m_fileStreamer;
};