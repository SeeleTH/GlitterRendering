#include "NDX11RenderTargetManager.h"

NDX11RenderTargetManager::NDX11RenderTargetManager(NDX11* dev)
	: m_pDevice(dev)
	, m_pMainRTV(0)
	, m_pMainDSV(0)
{
}

NDX11RenderTargetManager::~NDX11RenderTargetManager()
{
}


void NDX11RenderTargetManager::VInit()
{
	m_pMainRTV = new NTexture2DRes();
	m_pMainDSV = new NTexture2DRes();

	for (UINT32 i = 0; i < N_G_MAX_RENDERTARGETVIEWS; i++)
		m_pRTVs[i] = NULL;
	m_pDSV = NULL;
	m_u32ActiveRTVsNumber = 0;
}

void NDX11RenderTargetManager::VDestroy()
{
	N_DELETE(m_pMainRTV);
	N_DELETE(m_pMainDSV);
}


void NDX11RenderTargetManager::VActiveRenderTargets(NTexture2DRes** rtvs, NTexture2DRes* dsv, UINT32 size)
{
	N_ASSERT(m_pDevice);

	for (UINT32 i = 0; i < N_G_MAX_RENDERTARGETVIEWS; i++)
		m_pRTVs[i] = (i < size) ? rtvs[i] : NULL;

	m_pDSV = dsv;

	m_u32ActiveRTVsNumber = size;

	ActiveRenderTargets();
}


void NDX11RenderTargetManager::VClearRenderTargets(BOOL clearRTV, BOOL clearDSV)
{
	N_ASSERT(m_pDevice);

	if (clearRTV)
	{
		for (int i = 0; i < N_G_MAX_RENDERTARGETVIEWS; i++)
			m_pRTVs[i] = NULL;
		m_u32ActiveRTVsNumber = 0;
	}

	if (clearDSV)
	{
		m_pDSV = NULL;
	}

	ActiveRenderTargets();
}

void NDX11RenderTargetManager::VActiveMainRenderTarget()
{
	N_ASSERT(m_pDevice);

	m_pRTVs[0] = m_pMainRTV;
	for (int i = 1; i < N_G_MAX_RENDERTARGETVIEWS; i++)
		m_pRTVs[i] = NULL;
	m_u32ActiveRTVsNumber = 1;

	m_pDSV = m_pMainDSV;

	ActiveRenderTargets();
}


void NDX11RenderTargetManager::ActiveRenderTargets()
{
	N_ASSERT(m_pDevice);
	ID3D11RenderTargetView* pRTVs[N_G_MAX_RENDERTARGETVIEWS];
	for (int i = 0; i < N_G_MAX_RENDERTARGETVIEWS; i++)
		pRTVs[i] = (m_pRTVs) ? m_pRTVs[i]->GetRTV() : NULL;

	m_pDevice->SetRenderTargets(pRTVs, (m_pDSV) ? m_pDSV->GetDSV() : NULL, m_u32ActiveRTVsNumber);
}