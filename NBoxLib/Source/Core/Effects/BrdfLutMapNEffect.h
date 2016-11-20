#include "../../Macro/Macro.h"
#include "../../Graphics/NGraphics.h"
#include <memory>

class NResCache;
class NResHandle;

class BrdfLutMapNEffect : public INDX11Effect
{
public:
	BrdfLutMapNEffect(NGraphicsDevice* dev, NResCache* resCache, std::string filepath
		,BOOL writeToFile = true);
	~BrdfLutMapNEffect();

	void VApplyToQueue(NRenderCmdList* list) override;

	inline const NTexture2DRes* GetBrdfLutMap() { return m_pBrdfLutMap; }

protected:
	static const UINT32 g_u32BrdfLutBlockSizeX;
	static const UINT32 g_u32BrdfLutBlockSizeY;
	static const UINT32 g_u32BrdfLutSize;

	std::string m_sFilepath;
	BOOL m_bIsWriteToFile;
	BOOL m_bIsBrdfLoaded;
	BOOL m_bIsBrdfLutGenerateCmdIssued;
	NTexture2DRes* m_pBrdfLutMap;
	std::shared_ptr<NResHandle> m_pBrdfLutCS;
	std::shared_ptr<NResHandle> m_pBrdfLutMapTex;
};

class PrefilterEnvMapNEffect : public INDX11Effect
{
public:
	PrefilterEnvMapNEffect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap
		, std::string output);
	~PrefilterEnvMapNEffect();

	void VApplyToQueue(NRenderCmdList* list) override;

protected:
	enum {
		CUBEMAP_SIDE = 6
	};

	static const UINT32 g_u32BlockSizeX;
	static const UINT32 g_u32BlockSizeY;
	static const UINT32 g_u32BlockSizeZ;

	std::string m_sFilepath;
	BOOL m_bIsFinished;
	BOOL m_bIsCmdIssued;
	std::shared_ptr<NResHandle> m_pPrefilterCS;
	std::shared_ptr<NResHandle> m_pSourceTex;
	NTexture2DRes* m_pMainResult;
	std::vector<NTexture2DRes*> m_pUAVResult;

	struct CPPEROBJECT
	{
		float m_fTargetSize;
		float m_fRoughness;
		float m_fPadding[2];
		CPPEROBJECT(float size, float roughness) : m_fTargetSize(size), m_fRoughness(roughness){}
	};

	std::vector<CPPEROBJECT> m_cDataPerObject;
	NConstantBuffer m_cBufferPerObject;
};
