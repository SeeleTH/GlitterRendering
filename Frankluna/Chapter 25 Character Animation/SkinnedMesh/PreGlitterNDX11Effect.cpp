#include "PreGlitterNDX11Effect.h"

#include <sstream>

#include "../../../NBoxLib/Source/Graphics/NRenderCmd.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NShaderNResLoader.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"

const UINT32 GenFlakesMapNDX11Effect::g_u32BlockSizeX = 1024;
const UINT32 GenFlakesMapNDX11Effect::g_u32BlockSizeY = 1;
const UINT32 GenFlakesMapNDX11Effect::g_u32BlockSizeZ = 1;
GenFlakesMapNDX11Effect* GenFlakesMapNDX11Effect::gMainEffect = NULL;


GenFlakesMapNDX11Effect::GenFlakesMapNDX11Effect(NGraphicsDevice* dev, NResCache* resCache
	, NTexture2DProxy* normalMap, std::string output)
	: m_sFilepath(output)
	, m_bIsFinished(false)
	, m_bIsCmdIssued(false)
	, m_pSourceTex(normalMap)
{
	gMainEffect = this;

	NRes NormalToFlakesCsFile("FX\\GenFlakesMapFromNormalMap_cs.cso");
	m_pNormalMapToFlakesMapCS = resCache->GetHandle(&NormalToFlakesCsFile);
	NRes PrefilterFlakesCsFile("FX\\PrefilterFlakesMap_cs.cso");
	m_pPrefilterFlakesMapCS = resCache->GetHandle(&PrefilterFlakesCsFile);
	NRes GenFlakesCsFile("FX\\GenFlakeMapFromRandom_cs.cso");
	m_pGenFlakesMapCS = resCache->GetHandle(&GenFlakesCsFile);

	N_G_TEXTURE2D_DESC sourceDesc;
	if (m_pSourceTex)
	{
		m_pSourceTex->GetTexture()->GetDesc(sourceDesc);
	}
	else
	{
		sourceDesc.Width = 1024;
		sourceDesc.Height = 1024;
	}
	UINT32 maxSize = max(sourceDesc.Height, sourceDesc.Width);
	UINT32 maxMip = (UINT32)ceil(log2(maxSize));

	m_pMainResult = new NTexture2DRes();
	N_G_TEXTURE2D_DESC mainDesc;
	mainDesc.Width = sourceDesc.Width;
	mainDesc.Height = sourceDesc.Height;
	mainDesc.MipLevels = maxMip + 1;
	mainDesc.ArraySize = 1;
	mainDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	mainDesc.SampleDesc.Count = 1;
	mainDesc.SampleDesc.Quality = 0;
	mainDesc.Usage = D3D11_USAGE_DEFAULT;
	mainDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	mainDesc.CPUAccessFlags = 0;
	mainDesc.MiscFlags = 0;
	dev->CreateTextureFromDesc(*m_pMainResult, mainDesc, NULL, 0);
	dev->CreateSRV(*m_pMainResult);

	// ==========
	// TempTex
	// ==========
	N_G_TEXTURE2D_DESC tempTexDesc;
	tempTexDesc.Width = sourceDesc.Width;
	tempTexDesc.Height = sourceDesc.Height;
	tempTexDesc.MipLevels = 1;
	tempTexDesc.ArraySize = 1;
	tempTexDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	tempTexDesc.SampleDesc.Count = 1;
	tempTexDesc.SampleDesc.Quality = 0;
	tempTexDesc.Usage = D3D11_USAGE_DEFAULT;
	tempTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	tempTexDesc.CPUAccessFlags = 0;
	tempTexDesc.MiscFlags = 0;
	for (UINT32 i = 0; i < 2; i++)
	{
		m_pTempTex[i] = new NTexture2DRes();
		dev->CreateTextureFromDesc(*m_pTempTex[i], tempTexDesc, NULL, 0);
		dev->CreateSRV(*m_pTempTex[i]);
		dev->CreateUAV(*m_pTempTex[i]);
	}
	// ==========


	for (UINT32 mip = 0; maxSize > 0; mip++)
	{
		NTexture2DRes* flakestex;
		flakestex = new NTexture2DRes();
		flakestex->SetTexture(m_pMainResult->GetTexture());
		N_G_UNORDERED_ACCESS_VIEW_DESC flakesUAVDesc;
		flakesUAVDesc.Format = mainDesc.Format;
		flakesUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		flakesUAVDesc.Texture2D.MipSlice = mip;
		dev->CreateUAV(*flakestex, flakesUAVDesc);
		flakestex->SetTexture(nullptr);
		m_pUAVResult.push_back(flakestex);
		m_cDataPerObject.push_back(CPPEROBJECT((float)maxSize, (float)maxSize, mip));
		maxSize /= 2;
	}

	dev->CreateConstantBuffer(m_cBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CPPEROBJECT), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

GenFlakesMapNDX11Effect::~GenFlakesMapNDX11Effect()
{
	N_DELETE(m_pMainResult);

	for (auto it = m_pUAVResult.begin(); it != m_pUAVResult.end(); it++)
	{
		N_DELETE((*it));
	}
	m_pUAVResult.clear();

	for (UINT32 i = 0; i < 2; i++)
	{
		N_DELETE(m_pTempTex[i]);
	}
}

void GenFlakesMapNDX11Effect::VApplyToQueue(NRenderCmdList* list)
{
	if (m_bIsCmdIssued)
	{
		m_bIsFinished = true;
		for (UINT32 i = 0; i < 2; i++)
		{
			N_DELETE(m_pTempTex[i]);
		}
	}

	if (!m_bIsFinished && !m_bIsCmdIssued)
	{
		m_bIsCmdIssued = true;
		UINT32 texSlot = 0;

		if (m_pSourceTex)
			list->CmdSetComputeShader(((NComputeShaderNResExtraData*)m_pNormalMapToFlakesMapCS->GetExtra().get())->GetShader());
		else
			list->CmdSetComputeShader(((NComputeShaderNResExtraData*)m_pGenFlakesMapCS->GetExtra().get())->GetShader());
		list->CmdSetCSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
		if (m_pSourceTex)
			list->CmdSetCSStructureResource(0, m_pSourceTex->GetTexture());

		list->CmdSetConstantBufferData(&m_cBufferPerObject, &m_cDataPerObject[0], sizeof(CPPEROBJECT));
		list->CmdSetCSConstantBuffer(0, 1, &m_cBufferPerObject);
		list->CmdSetCSTextureUnorderedAccessView(0, m_pUAVResult[texSlot]);
		list->CmdSetCSTextureUnorderedAccessView(1, m_pTempTex[texSlot]);
		texSlot = !texSlot;
		list->CmdDispatch((UINT32)ceil(m_cDataPerObject[0].m_fTargetSize.x * m_cDataPerObject[0].m_fTargetSize.y / (float)g_u32BlockSizeX)
			, 1, 1);

		list->CmdSetCSTextureUnorderedAccessView(0, NULL);
		list->CmdSetCSTextureUnorderedAccessView(1, NULL);
		list->CmdSetComputeShader(NULL);

		list->CmdSetComputeShader(((NComputeShaderNResExtraData*)m_pPrefilterFlakesMapCS->GetExtra().get())->GetShader());

		for (UINT32 mip = 1; mip < m_cDataPerObject.size(); mip++)
		{
			list->CmdSetConstantBufferData(&m_cBufferPerObject, &m_cDataPerObject[mip], sizeof(CPPEROBJECT));
			list->CmdSetCSConstantBuffer(0, 1, &m_cBufferPerObject);
			list->CmdSetCSTextureResource(0, m_pTempTex[!texSlot]);
			list->CmdSetCSTextureUnorderedAccessView(0, m_pUAVResult[mip]);
			list->CmdSetCSTextureUnorderedAccessView(1, m_pTempTex[texSlot]);
			texSlot = !texSlot;
			UINT32 dispatchN = (UINT32)ceil(m_cDataPerObject[mip].m_fTargetSize.x * m_cDataPerObject[mip].m_fTargetSize.y / (float)g_u32BlockSizeX);
				list->CmdDispatch(dispatchN
				, 1, 1);
			list->CmdSetCSTextureResource(0, NULL);
			list->CmdSetCSTextureUnorderedAccessView(0, NULL);
			list->CmdSetCSTextureUnorderedAccessView(1, NULL);
		}

		list->CmdSetComputeShader(NULL);

		list->CmdWriteTextureToDDS(m_pMainResult, m_sFilepath);
	}
}

const float GenFlakesMapNDX11Effect::PI = 3.14159265358979;

void GenFlakesMapNDX11Effect::PrintVectorFlagList(UINT32 flagBitX, UINT32 flagBitY)
{
	for (UINT32 i = 0; i < flagBitX * flagBitY; i++)
	{
		DirectX::XMFLOAT3 result;
		UINT32 vert = i / flagBitY;
		UINT32 hori = i - vert * flagBitY;
		float x_axis_angle = ((float)vert + 0.5f) / (float)flagBitX * 0.5f * PI;
		float y_axis_angle = ((float)hori + 0.5f) / (float)flagBitY * 2.f * PI - PI;
		result.x = cos(x_axis_angle) * sin(y_axis_angle);
		result.y = sin(x_axis_angle);
		result.z = cos(x_axis_angle) * cos(y_axis_angle);
		DirectX::XMVECTOR resultVec = DirectX::XMLoadFloat3(&result);
		resultVec = DirectX::XMVector3Normalize(resultVec);
		DirectX::XMStoreFloat3(&result, resultVec);

		std::wstringstream sstm;
		sstm << L"float3(" << result.x << "f, " << result.y << "f, " << result.z << L"f),\n";
		std::wstring str = sstm.str();
		OutputDebugString(str.c_str());
	}
}