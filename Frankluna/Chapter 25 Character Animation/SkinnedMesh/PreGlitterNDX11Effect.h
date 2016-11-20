#include "../../../NBoxLib/Source/Macro/Macro.h"
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include <memory>

class NResCache;
class NResHandle;

class GenFlakesMapNDX11Effect : public INDX11Effect
{
public:
	GenFlakesMapNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, NTexture2DProxy* normalMap
		, std::string output);
	~GenFlakesMapNDX11Effect();

	void VApplyToQueue(NRenderCmdList* list) override;
	
	inline NTexture2DRes* GetMainResult() { return m_pMainResult; }

	static void PrintVectorFlagList(UINT32 flagBitX, UINT32 flagBitY);


	static GenFlakesMapNDX11Effect* gMainEffect;

protected:
	static const float PI;

	static const UINT32 g_u32BlockSizeX;
	static const UINT32 g_u32BlockSizeY;
	static const UINT32 g_u32BlockSizeZ;

	std::string m_sFilepath;
	BOOL m_bIsFinished;
	BOOL m_bIsCmdIssued;
	std::shared_ptr<NResHandle> m_pGenFlakesMapCS;
	std::shared_ptr<NResHandle> m_pNormalMapToFlakesMapCS;
	std::shared_ptr<NResHandle> m_pPrefilterFlakesMapCS;
	NTexture2DProxy* m_pSourceTex;
	NTexture2DRes* m_pMainResult;
	std::vector<NTexture2DRes*> m_pUAVResult;
	NTexture2DRes* m_pTempTex[2];

	struct CPPEROBJECT
	{
		DirectX::XMFLOAT2 m_fTargetSize;
		UINT32 m_uMipLevel;
		float m_fPadding;
		CPPEROBJECT(float sizex, float sizey, UINT32 mip)
		{ 
			m_fTargetSize.x = sizex; 
			m_fTargetSize.y = sizey; 
			m_uMipLevel = mip;
		}
	};

	std::vector<CPPEROBJECT> m_cDataPerObject;
	NConstantBuffer m_cBufferPerObject;
};
