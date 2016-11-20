#include "BrdfLutMapNEffect.h"

#include "../../Graphics/NRenderCmd.h"
#include "../../Resource/NResCache.h"
#include "../ResLoaders/NShaderNResLoader.h"
#include "../ResLoaders/NTextureNResLoader.h"

const UINT32 BrdfLutMapNEffect::g_u32BrdfLutBlockSizeX = 1024;
const UINT32 BrdfLutMapNEffect::g_u32BrdfLutBlockSizeY = 1;
const UINT32 BrdfLutMapNEffect::g_u32BrdfLutSize = 1024;

BrdfLutMapNEffect::BrdfLutMapNEffect(NGraphicsDevice* dev, NResCache* resCache, std::string filepath
	, BOOL writeToFile)
	: m_sFilepath(filepath)
	, m_bIsWriteToFile(writeToFile)
	, m_bIsBrdfLoaded(false)
	, m_bIsBrdfLutGenerateCmdIssued(false)
	, m_pBrdfLutMap(nullptr)
{
	NRes brdfLutMapTexRes(m_sFilepath);
	m_pBrdfLutMapTex = resCache->GetHandle(&brdfLutMapTexRes);
	if (m_pBrdfLutMapTex.get() == NULL)
	{
		NRes brdfLutGenCsFile("FX\\BRDFLUT_cs.cso");
		m_pBrdfLutCS = resCache->GetHandle(&brdfLutGenCsFile);
		m_pBrdfLutMap = new NTexture2DRes();
		N_G_TEXTURE2D_DESC texDesc;
		texDesc.Width = g_u32BrdfLutSize;
		texDesc.Height = g_u32BrdfLutSize;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16_FLOAT;//DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		dev->CreateTextureFromDesc(*m_pBrdfLutMap, texDesc, NULL, 0);

		dev->CreateSRV(*m_pBrdfLutMap);
		dev->CreateUAV(*m_pBrdfLutMap);
	}
	else
	{
		m_bIsBrdfLoaded = true;
		m_pBrdfLutMap = ((NDDSTextureNResExtraData*)m_pBrdfLutMapTex->GetExtra().get())->GetTexture();
	}
}

BrdfLutMapNEffect::~BrdfLutMapNEffect()
{
	if (m_pBrdfLutMapTex.get() == NULL)
	{
		N_DELETE(m_pBrdfLutMap);
	}
}

void BrdfLutMapNEffect::VApplyToQueue(NRenderCmdList* list)
{
	if (m_bIsBrdfLutGenerateCmdIssued)
		m_bIsBrdfLoaded = true;

	if (!m_bIsBrdfLoaded && !m_bIsBrdfLutGenerateCmdIssued)
	{
		m_bIsBrdfLutGenerateCmdIssued = true;
		list->CmdSetComputeShader(((NComputeShaderNResExtraData*)m_pBrdfLutCS->GetExtra().get())->GetShader());
		list->CmdSetCSTextureUnorderedAccessView(0, m_pBrdfLutMap);
		list->CmdDispatch(g_u32BrdfLutSize / g_u32BrdfLutBlockSizeX, g_u32BrdfLutSize / g_u32BrdfLutBlockSizeY, 1);
		list->CmdSetCSTextureUnorderedAccessView(0, NULL);
		list->CmdSetComputeShader(NULL);
		if (m_bIsWriteToFile)
			list->CmdWriteTextureToDDS(m_pBrdfLutMap, m_sFilepath);
	}
}

const UINT32 PrefilterEnvMapNEffect::g_u32BlockSizeX = 128;
const UINT32 PrefilterEnvMapNEffect::g_u32BlockSizeY = 1;
const UINT32 PrefilterEnvMapNEffect::g_u32BlockSizeZ = 6;

PrefilterEnvMapNEffect::PrefilterEnvMapNEffect(NGraphicsDevice* dev, NResCache* resCache
	, std::shared_ptr<NResHandle> cubeMap, std::string output)
	: m_sFilepath(output)
	, m_bIsFinished(false)
	, m_bIsCmdIssued(false)
	, m_pSourceTex(cubeMap)
{
	NRes PreFilterEnvCsFile("FX\\BRDFEnvPreFilter_cs.cso");
	m_pPrefilterCS = resCache->GetHandle(&PreFilterEnvCsFile);

	N_G_TEXTURE2D_DESC sourceDesc;
	((NDDSTextureNResExtraData*)m_pSourceTex->GetExtra().get())->GetTexture()->GetDesc(sourceDesc);
	UINT32 maxSize = max(sourceDesc.Height, sourceDesc.Width);
	UINT32 maxMip = (UINT32)ceil(log2(maxSize));

	m_pMainResult = new NTexture2DRes();
	N_G_TEXTURE2D_DESC mainDesc;
	mainDesc.Width = sourceDesc.Width;
	mainDesc.Height = sourceDesc.Height;
	mainDesc.MipLevels = maxMip + 1;
	mainDesc.ArraySize = 6;
	mainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	mainDesc.SampleDesc.Count = 1;
	mainDesc.SampleDesc.Quality = 0;
	mainDesc.Usage = D3D11_USAGE_DEFAULT;
	mainDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	mainDesc.CPUAccessFlags = 0;
	mainDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	dev->CreateTextureFromDesc(*m_pMainResult, mainDesc, NULL, 0);
	dev->CreateSRV(*m_pMainResult);


	for (UINT32 mip = 0; maxSize > 0; mip++)
	{
		NTexture2DRes* cubetex;
		cubetex = new NTexture2DRes();
		cubetex->SetTexture(m_pMainResult->GetTexture());
		N_G_UNORDERED_ACCESS_VIEW_DESC cubeUAVDesc;
		cubeUAVDesc.Format = mainDesc.Format;
		cubeUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		cubeUAVDesc.Texture2DArray.ArraySize = 6;
		cubeUAVDesc.Texture2DArray.FirstArraySlice = 0;
		cubeUAVDesc.Texture2DArray.MipSlice = mip;
		dev->CreateUAV(*cubetex, cubeUAVDesc);
		cubetex->SetTexture(nullptr);
		m_pUAVResult.push_back(cubetex);
		m_cDataPerObject.push_back(CPPEROBJECT((float)maxSize, (float)mip / (float)maxMip));
		maxSize /= 2;
	}

	dev->CreateConstantBuffer(m_cBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CPPEROBJECT), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

PrefilterEnvMapNEffect::~PrefilterEnvMapNEffect()
{
	N_DELETE(m_pMainResult);

	for (auto it = m_pUAVResult.begin(); it != m_pUAVResult.end(); it++)
	{
			N_DELETE((*it));
	}
	m_pUAVResult.clear();
}

void PrefilterEnvMapNEffect::VApplyToQueue(NRenderCmdList* list)
{
	if (m_bIsCmdIssued)
		m_bIsFinished = true;

	if (!m_bIsFinished && !m_bIsCmdIssued)
	{
		m_bIsCmdIssued = true;
		list->CmdSetComputeShader(((NComputeShaderNResExtraData*)m_pPrefilterCS->GetExtra().get())->GetShader());
		list->CmdSetCSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
		list->CmdSetCSStructureResource(0, ((NDDSTextureNResExtraData*)m_pSourceTex->GetExtra().get())->GetTexture());

		for (UINT32 mip = 0; mip < m_cDataPerObject.size(); mip++)
		{
			float mipSize = m_cDataPerObject[mip].m_fTargetSize;
			list->CmdSetConstantBufferData(&m_cBufferPerObject, &m_cDataPerObject[mip], sizeof(CPPEROBJECT));
			list->CmdSetCSConstantBuffer(0, 1, &m_cBufferPerObject);
			list->CmdSetCSTextureUnorderedAccessView(0, m_pUAVResult[mip]);
			list->CmdDispatch((UINT32)ceil(mipSize * mipSize / (float)g_u32BlockSizeX) //ceil(mipSize / (float)g_u32BlockSizeX)
				, 1 //ceil(mipSize / (float)g_u32BlockSizeY)
				, CUBEMAP_SIDE / g_u32BlockSizeZ);
		}

		list->CmdSetCSTextureUnorderedAccessView(0, NULL);
		list->CmdSetComputeShader(NULL);

		list->CmdWriteTextureToDDS(m_pMainResult, m_sFilepath);
	}
}