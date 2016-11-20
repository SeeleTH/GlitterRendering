#pragma once

//#define FORWARDRENDER
#define DEFERREDRENDER

#include "../../../NBoxLib/Source/Graphics/NRenderManager.h"

#ifdef FORWARDRENDER
class SkyNDX11Effect;
class NormalNDX11Effect;
#endif
#ifdef DEFERREDRENDER
class PBRNDX11Effect;
#endif
class NResCache;
class Camera;

class SkinnedMeshRenderManager : public NRenderManager
{
public:
	SkinnedMeshRenderManager(NGraphicsDevice* dev, NResCache* resCache, Camera* cam);

	virtual void VInit() override;
	virtual void VRender() override;
	virtual void VDestroy() override;

protected:
	Camera* m_pCam;

	NResCache* m_pResCache;

#ifdef FORWARDRENDER
	SkyNDX11Effect* m_pSkyEffect;
	NormalNDX11Effect* m_pNormalEffect;
#endif
#ifdef DEFERREDRENDER
	PBRNDX11Effect* m_pPBREffect;
#endif
};