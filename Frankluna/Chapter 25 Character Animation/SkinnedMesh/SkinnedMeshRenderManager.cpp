#include "SkinnedMeshRenderManager.h"
#include "SkyNDX11Effect.h"

#ifdef FORWARDRENDER
#include "NormalNDX11Effect.h"
#else
#include "PBRNDX11Effect.h"
#endif

#include "../../Common/Camera.h"

#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"

SkinnedMeshRenderManager::SkinnedMeshRenderManager(NGraphicsDevice* dev, NResCache* resCache, Camera* cam)
	: NRenderManager(dev)
	, m_pResCache(resCache)
#ifdef FORWARDRENDER
	, m_pSkyEffect(nullptr)
	, m_pNormalEffect(nullptr)
#endif
#ifdef DEFERREDRENDER
	, m_pPBREffect(nullptr)
#endif
	, m_pCam(cam)
{
}

void SkinnedMeshRenderManager::VInit()
{
	NRenderManager::VInit();

	NRes cubeFile("Textures\\uffizi_cross.dds");
	std::shared_ptr<NResHandle> cubeFileHandle = m_pResCache->GetHandle(&cubeFile);


#ifdef FORWARDRENDER
	m_pSkyEffect = new SkyNDX11Effect(m_pDevice, m_pResCache, cubeFileHandle);
	DirectX::XMMATRIX tempMatProj(*m_pCam->Proj().m);
	m_pSkyEffect->VSetProjection(tempMatProj);

	m_pNormalEffect = new NormalNDX11Effect(m_pDevice, m_pResCache, cubeFileHandle);
	m_pNormalEffect->VSetProjection(tempMatProj);
#endif
#ifdef DEFERREDRENDER
	m_pPBREffect = new PBRNDX11Effect(m_pDevice, m_pResCache, cubeFileHandle);
	DirectX::XMMATRIX tempMatProj(*m_pCam->Proj().m);
	m_pPBREffect->VSetProjection(tempMatProj);
	m_pPBREffect->SetCamNearZFarZ(m_pCam->GetNearZ(), m_pCam->GetFarZ());
#endif
}

void SkinnedMeshRenderManager::VRender()
{
	DirectX::XMMATRIX tempMatView(*m_pCam->View().m);
	NRenderCmdList cmdList;
	// Clear cmd
	cmdList.CmdClear(N_B_ALL, NGraphicsDevice::g_pMainGraphicsDevice->GetMainRenderTargets(), NGraphicsDevice::g_pMainGraphicsDevice->GetMainDepthStencil());
	cmdList.CmdSetDefViewports();

	cmdList.CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_equalsDSS, 0);
	cmdList.CmdSetRasterizerState(NULL);
	//cmdList.CmdSetRasterizerState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_wireframeRS);

#ifdef FORWARDRENDER
	m_pNormalEffect->VSetWorld(DirectX::XMMatrixTranslation(0.f, 0.f, 0.f));
	m_pNormalEffect->VSetView(tempMatView);
	m_pNormalEffect->VApplyToQueue(&cmdList);

	m_pSkyEffect->VSetWorld(DirectX::XMMatrixTranslation(m_pCam->GetPosition().x, m_pCam->GetPosition().y, m_pCam->GetPosition().z));
	m_pSkyEffect->VSetView(tempMatView);
	m_pSkyEffect->VApplyToQueue(&cmdList);
#endif
#ifdef DEFERREDRENDER
	m_pPBREffect->SetCamPos(m_pCam->GetPosition().x, m_pCam->GetPosition().y, m_pCam->GetPosition().z);
	m_pPBREffect->VSetWorld(DirectX::XMMatrixTranslation(0.f, 0.f, 0.f));
	m_pPBREffect->VSetView(tempMatView);
	m_pPBREffect->VApplyToQueue(&cmdList);
#endif

	// Present
	cmdList.CmdPresent();

	m_pRenderQueue->QueueCommandList(cmdList);

	// Actually this should sent command to render queues thread..
	m_pRenderQueue->VRender();
}

void SkinnedMeshRenderManager::VDestroy()
{
	NRenderManager::VDestroy();


#ifdef FORWARDRENDER
	N_DELETE(m_pSkyEffect);
	N_DELETE(m_pNormalEffect);
#endif
#ifdef DEFERREDRENDER
	N_DELETE(m_pPBREffect);
#endif
}