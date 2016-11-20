#pragma once

#include "../Macro/Macro.h"
#include "NDX11.h"
#include "NTexture2DRes.h"


class NDX11RenderTargetManager
{
	friend class NDX11;

public:
	NDX11RenderTargetManager(NDX11* dev);
	~NDX11RenderTargetManager();

	virtual void VInit();
	virtual void VDestroy();

	virtual void VActiveRenderTargets(NTexture2DRes** rtvs, NTexture2DRes* dsv, UINT32 size);
	virtual void VClearRenderTargets(BOOL clearRTV, BOOL clearDSV);
	virtual void VActiveMainRenderTarget();

	inline virtual NTexture2DRes* VGetRTV(UINT32 slot) { N_ASSERT(slot < N_G_MAX_RENDERTARGETVIEWS); return m_pRTVs[slot]; }
	inline virtual NTexture2DRes* VGetDSV() { return m_pDSV; }

	inline virtual NTexture2DRes* VGetMainRTV() { return m_pMainRTV; }
	inline virtual NTexture2DRes* VGetMainDSV() { return m_pMainDSV; }

protected:

	void ActiveRenderTargets();

	NDX11* m_pDevice;

	NTexture2DRes* m_pRTVs[N_G_MAX_RENDERTARGETVIEWS];
	NTexture2DRes* m_pDSV;
	UINT32 m_u32ActiveRTVsNumber;

	NTexture2DRes* m_pMainRTV;
	NTexture2DRes* m_pMainDSV;
};