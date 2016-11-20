#include "PBRNDX11Effect.h"

#include "SkinnedMeshDemo.h"

#include "../../../NBoxLib/Source/Graphics/NRenderCmd.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NShaderNResLoader.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"

#include "../../../NBoxLib/Source/Core/Effects/BrdfLutMapNEffect.h"

#ifdef GEN_FLAKES_MAP
#include "PreGlitterNDX11Effect.h"
#endif

#include "../../Common/GeometryGenerator.h"

PBRNDX11Effect::PBRNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap)
{
	m_u32CurrentLightAccumInd = 0;
	m_pDefTexture = m_pDefNormalTexture = m_pDefFlakeTexture = NULL;

	N_G_SHADER_LAYOUT layout;
	layout.numElements = 4;
	layout.layouts[0].SemanticName = "POSITION";
	layout.layouts[0].SemanticIndex = 0;
	layout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout.layouts[0].InputSlot = 0;
	layout.layouts[0].AlignedByteOffset = 0;
	layout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout.layouts[0].InstanceDataStepRate = 0;

	layout.layouts[1].SemanticName = "NORMAL";
	layout.layouts[1].SemanticIndex = 0;
	layout.layouts[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout.layouts[1].InputSlot = 0;
	layout.layouts[1].AlignedByteOffset = 12;
	layout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout.layouts[1].InstanceDataStepRate = 0;

	layout.layouts[2].SemanticName = "TEXCOORD";
	layout.layouts[2].SemanticIndex = 0;
	layout.layouts[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	layout.layouts[2].InputSlot = 0;
	layout.layouts[2].AlignedByteOffset = 24;
	layout.layouts[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout.layouts[2].InstanceDataStepRate = 0;

	layout.layouts[3].SemanticName = "TANGENT";
	layout.layouts[3].SemanticIndex = 0;
	layout.layouts[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout.layouts[3].InputSlot = 0;
	layout.layouts[3].AlignedByteOffset = 32;
	layout.layouts[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout.layouts[3].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasic_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(layout);

	NRes psFile("FX\\DeferredBasic_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	m_pBrdfLutMapEffect = new BrdfLutMapNEffect(dev, resCache, "Textures\\brdflutmap.dds");


#ifdef GEN_PREFILTER_ENVMAP
	m_pPrefilterEnvMapEffect = new PrefilterEnvMapNEffect(dev, resCache, cubeMap, "Textures\\prefilteredmap.dds");
#endif

	// Generate vertex and index buffer
	BuildBuffers(dev);

	for (UINT32 i = 0; i < GBUFFERS_NO; i++)
		m_pGBuffers[i] = NULL;
	BuildGBuffers(*dev);

	VSetWorld(DirectX::XMMatrixIdentity());
	VSetView(DirectX::XMMatrixIdentity());
	VSetProjection(DirectX::XMMatrixIdentity());


#ifdef GEN_FLAKES_MAP
	m_pGenFlakesMapEffect = new GenFlakesMapNDX11Effect(dev, resCache
#ifdef GEN_FLAKES_MAP_RANDOM
		, nullptr
#else
		, SkinnedMeshApp::Instance().GetTexturePool()->GetTexture(GEN_FLAKES_MAP_NORMAL,0U,0)
#endif
		, GEN_FLAKES_MAP_OUTPUT
		);
#endif

	// =================
	// OBJECTS' MATRICES
	// =================

	RENDERITEM gridData;
	RENDERITEM boxData;
	RENDERITEM cylinderData[10];
	RENDERITEM sphereData[10];

	DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&gridData.m_matWorld, I);

	DirectX::XMMATRIX boxScale = DirectX::XMMatrixScaling(3.0f, 1.0f, 3.0f);
	DirectX::XMMATRIX boxOffset = DirectX::XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	DirectX::XMStoreFloat4x4(&boxData.m_matWorld, DirectX::XMMatrixMultiply(boxScale, boxOffset));

	for (UINT32 i = 0; i < 5; ++i)
	{
		DirectX::XMStoreFloat4x4(&cylinderData[i * 2 + 0].m_matWorld, DirectX::XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		DirectX::XMStoreFloat4x4(&cylinderData[i * 2 + 1].m_matWorld, DirectX::XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		DirectX::XMStoreFloat4x4(&sphereData[i * 2 + 0].m_matWorld, DirectX::XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		DirectX::XMStoreFloat4x4(&sphereData[i * 2 + 1].m_matWorld, DirectX::XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	DirectX::XMStoreFloat4x4(&boxData.m_buffer.m_matTexTransform, DirectX::XMMatrixScaling(2.0f, 1.0f, 1.0f));
	DirectX::XMStoreFloat4x4(&gridData.m_buffer.m_matTexTransform, DirectX::XMMatrixScaling(8.f, 10.f, 1.f));
	for (UINT32 i = 0; i < 10; i++)
	{
		DirectX::XMStoreFloat4x4(&sphereData[i].m_buffer.m_matTexTransform, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&cylinderData[i].m_buffer.m_matTexTransform, DirectX::XMMatrixScaling(1.0f, 2.0f, 1.0f));
	}

	gridData.m_uIndexCount = m_u32GridIndexCount;
	gridData.m_uIndexOffset = m_u32GridIndexOffset;
	gridData.m_uVertexOffset = m_u32GridVertexOffset;
	gridData.m_pDiffuseMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\floor.dds");
	gridData.m_pNormalMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\floor_nmap.dds");
	gridData.m_matBuffer.m_fRoughnessMultiplier = 0.35f;
	gridData.m_pFlakesMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\Flakes\\metallicflakes.dds", NTexture2DPool::TEXTURE2D_TYPE_UINT);
	//gridData.m_matBuffer.m_fFlakesDensityMultiplier = 10.f;
	gridData.m_matBuffer.m_f3AlbedoColorMultiplier.x = 0.3f;
	gridData.m_matBuffer.m_f3AlbedoColorMultiplier.y = 0.3f;
	gridData.m_matBuffer.m_f3AlbedoColorMultiplier.z = 0.3f;

	boxData.m_uIndexCount = m_u32BoxIndexCount;
	boxData.m_uIndexOffset = m_u32BoxIndexOffset;
	boxData.m_uVertexOffset = m_u32BoxVertexOffset;
	//boxData.m_pDiffuseMap = ((NDDSTextureNResExtraData*)m_pTextures[1]->GetExtra().get())->GetTexture();
	boxData.m_pNormalMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\bricks_nmap.dds");
	boxData.m_pFlakesMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\Flakes\\metallicflakes.dds", NTexture2DPool::TEXTURE2D_TYPE_UINT);
	boxData.m_matBuffer.m_fRoughnessMultiplier = 0.1f;
	boxData.m_matBuffer.m_f3AlbedoColorMultiplier.x = 1.0f;
	boxData.m_matBuffer.m_f3AlbedoColorMultiplier.y = 0.766f;
	boxData.m_matBuffer.m_f3AlbedoColorMultiplier.z = 0.336f;
	boxData.m_matBuffer.m_fMetallicMultiplier = 1.0f;
	boxData.m_matBuffer.m_fFlakesDensityMultiplier = 1.f;

	for (UINT32 i = 0; i < 10; i++)
	{
		cylinderData[i].m_uIndexCount = m_u32CylinderIndexCount;
		cylinderData[i].m_uIndexOffset = m_u32CylinderIndexOffset;
		cylinderData[i].m_uVertexOffset = m_u32CylinderVertexOffset;
		cylinderData[i].m_pDiffuseMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\bricks.dds");
		cylinderData[i].m_pNormalMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\bricks_nmap.dds");
		cylinderData[i].m_matBuffer.m_fRoughnessMultiplier = 0.5f;
		cylinderData[i].m_pFlakesMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\Flakes\\metallicflakes.dds", NTexture2DPool::TEXTURE2D_TYPE_UINT);
		cylinderData[i].m_matBuffer.m_fFlakesDensityMultiplier = 1.f;

		sphereData[i].m_uIndexCount = m_u32SphereIndexCount;
		sphereData[i].m_uIndexOffset = m_u32SphereIndexOffset;
		sphereData[i].m_uVertexOffset = m_u32SphereVertexOffset;
		sphereData[i].m_pDiffuseMap = NULL;
		//sphereData[i].m_pNormalMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\bricks_nmap.dds");
		sphereData[i].m_matBuffer.m_f3AlbedoColorMultiplier.x = 1.0f;
		sphereData[i].m_matBuffer.m_f3AlbedoColorMultiplier.y = 0.766f;
		sphereData[i].m_matBuffer.m_f3AlbedoColorMultiplier.z = 0.336f;
		sphereData[i].m_matBuffer.m_fMetallicMultiplier = 1.0f;
		sphereData[i].m_matBuffer.m_fRoughnessMultiplier = 0.1f + 0.9f * (float)i / 10.f;
		sphereData[i].m_pFlakesMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\Flakes\\metallicflakes.dds", NTexture2DPool::TEXTURE2D_TYPE_UINT);
		sphereData[i].m_matBuffer.m_fFlakesDensityMultiplier = 10.f;
	}

	m_sRenderItems.push_back(gridData);
	//m_sRenderItems.push_back(boxData);
	//for (UINT32 i = 0; i < 10; i++)
	//{
	//	m_sRenderItems.push_back(cylinderData[i]);
	//	m_sRenderItems.push_back(sphereData[i]);
	//}

	// Test Model
	{
		RENDERITEM_MODEL testModel;
		testModel.m_pModel = SkinnedMeshApp::Instance().GetGlitterModelPool()->GetModel("Models\\highheels_1.obj");
		//testModel.m_pDiffuseMap = NULL;
		//testModel.m_matBuffer.m_f3AlbedoColorMultiplier.x = 1.0f;
		//testModel.m_matBuffer.m_f3AlbedoColorMultiplier.y = 0.766f;
		//testModel.m_matBuffer.m_f3AlbedoColorMultiplier.z = 0.336f;
		//testModel.m_matBuffer.m_fMetallicMultiplier = 1.0f;
		//testModel.m_matBuffer.m_fRoughnessMultiplier = 0.2f;
		//testModel.m_pFlakesMap = SkinnedMeshApp::Instance().GetTexturePool()->GetTexture("Textures\\floorflake.dds", NTexture2DPool::TEXTURE2D_TYPE_UINT);
		//testModel.m_matBuffer.m_fFlakesDensityMultiplier = 50.f;
		m_sModelRenderItems.push_back(testModel);
	}

	m_vLights.push_back(new DeferredDirLight(DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f)
		, DirectX::XMFLOAT3(1.5f, 1.5f, 1.5f), true));
	//m_vLights.push_back(new DeferredDirLight(DirectX::XMFLOAT3(0.57735f, -0.2f, 0.57735f)
	//	, DirectX::XMFLOAT3(1.5f, 1.5f, 1.5f), true));

	for (auto it = m_vLights.begin(); it != m_vLights.end(); it++)
	{
		(*it)->VBuildBuffer(dev, resCache);
	}

	SSAOEffect* ssao = new SSAOEffect();
	m_vPostProcesses.push_back(new AmbientBRDFEffect());
	//m_vPostProcesses.push_back(new GlitterEffect());
	m_vPostProcesses.push_back(ssao);
	m_vPostProcesses.push_back(new BasicSkyEffect(cubeMap));
	m_vPostProcesses.push_back(new CompositeEffect(ssao));
	m_vPostProcesses.push_back(new CopyToBackBufferEffect());

	for (auto it = m_vPostProcesses.begin(); it != m_vPostProcesses.end(); it++)
	{
		(*it)->VBuildBuffer(dev, resCache);
	}

}

PBRNDX11Effect::~PBRNDX11Effect()
{
	m_sModelRenderItems.clear();
	m_sRenderItems.clear();
	N_DELETE(m_pDefTexture);
	N_DELETE(m_pDefNormalTexture);
	N_DELETE(m_pDefFlakeTexture);

	N_DELETE(m_pBrdfLutMapEffect);

	for (auto it = m_vLights.begin(); it != m_vLights.end(); it++)
	{
		N_DELETE((*it));
	}
	m_vLights.clear();

	for (auto it = m_vPostProcesses.begin(); it != m_vPostProcesses.end(); it++)
	{
		N_DELETE((*it));
	}
	m_vPostProcesses.clear();

#ifdef GEN_PREFILTER_ENVMAP
	N_DELETE(m_pPrefilterEnvMapEffect);
#endif
#ifdef GEN_FLAKES_MAP
	N_DELETE(m_pGenFlakesMapEffect);
#endif

	for (UINT32 i = 0; i < GBUFFERS_NO; i++)
	{
		N_DELETE(m_pGBuffers[i]);
	}

	for (UINT32 i = 0; i < LIGHTACCUM_NO; i++)
	{
		N_DELETE(m_pLightAccumBuffers[i]);
	}
}


void PBRNDX11Effect::VApplyToQueue(NRenderCmdList* list)
{
	//temp
	//for (auto it = m_vLights.begin(); it != m_vLights.end(); it++)
	//{
	//	DeferredDirLight* dirLight = (DeferredDirLight*)(*it);
	//	if (dirLight)
	//	{
	//		DirectX::XMVECTOR dirLightVec = dirLight->VGetLightDir();
	//		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(0.f, XM_PI*0.1f*SkinnedMeshApp::Instance().GetTime()->GetDeltaFloat(), 0.f);
	//		dirLightVec = DirectX::XMVector3Transform(dirLightVec, rotMat);
	//		dirLight->VSetLightDir(dirLightVec);
	//	}
	//}

	m_pBrdfLutMapEffect->VApplyToQueue(list);

#ifdef GEN_PREFILTER_ENVMAP
	m_pPrefilterEnvMapEffect->VApplyToQueue(list);
#endif
#ifdef GEN_FLAKES_MAP
	m_pGenFlakesMapEffect->VApplyToQueue(list);
#endif

	UpdateBuffers();

	list->CmdSetDefViewports();
	ClearGBuffers(*list);
	list->CmdSetRendertargets(GBUFFERS_NO - GBUFFERS_NORMALDEPTHROUGHNESS, m_pGBuffers + GBUFFERS_NORMALDEPTHROUGHNESS, NGraphicsDevice::g_pMainGraphicsDevice->GetMainDepthStencil());// m_pGBuffers[0]);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetVertexBuffer(0, 1, &m_defVertBuff);
	list->CmdSetIndexBuffer(&m_defIndBuff);
	list->CmdSetConstantBufferData(&m_cPSBufferPerFrame, &m_sPSBufferPerFrame, sizeof(CBPERFRAME_PS));
	list->CmdSetPSConstantBuffer(0, 1, &m_cPSBufferPerFrame);

	for (auto it = m_sRenderItems.begin(); it != m_sRenderItems.end(); it++)
	{
		list->CmdSetConstantBufferData(&m_cBufferPerObject, &it->m_buffer, sizeof(CBPEROBJECT_VS));
		list->CmdSetVSConstantBuffer(0, 1, &m_cBufferPerObject);
		list->CmdSetConstantBufferData(&m_cPSBufferPerObject, &it->m_matBuffer, sizeof(CBPEROBJECT_PS));
		list->CmdSetPSConstantBuffer(1, 1, &m_cPSBufferPerObject);
		list->CmdSetPSTextureResource(0, (it->m_pDiffuseMap) ? it->m_pDiffuseMap->GetTexture() : m_pDefTexture);
		list->CmdSetPSTextureResource(1, (it->m_pNormalMap) ? it->m_pNormalMap->GetTexture() : m_pDefNormalTexture);
		list->CmdSetPSTextureResource(2, (it->m_pRoughnessSpecularMetallicCavityMap) ? 
			it->m_pRoughnessSpecularMetallicCavityMap->GetTexture() : m_pDefTexture);
		list->CmdSetPSTextureResource(3, (it->m_pMatMaskMap) ? it->m_pMatMaskMap->GetTexture() : m_pDefTexture);
		list->CmdSetPSTextureResource(4, (it->m_pFlakesMap) ? it->m_pFlakesMap->GetTexture() : m_pDefFlakeTexture);
		list->CmdSetVertexBuffer(0, 1, (it->m_pVertBuff) ? it->m_pVertBuff : &m_defVertBuff);
		list->CmdSetIndexBuffer((it->m_pIndBuff) ? it->m_pIndBuff : &m_defIndBuff);
		list->CmdDrawIndexed(it->m_uIndexCount, it->m_uIndexOffset, it->m_uVertexOffset);
	}

	for (auto it = m_sModelRenderItems.begin(); it != m_sModelRenderItems.end(); it++)
	{
		if (it->m_pModel->GetWorld() == NULL)
			continue;

		list->CmdSetConstantBufferData(&m_cBufferPerObject, &it->m_buffer, sizeof(CBPEROBJECT_VS));
		list->CmdSetVSConstantBuffer(0, 1, &m_cBufferPerObject);
		//list->CmdSetConstantBufferData(&m_cPSBufferPerObject, &it->m_matBuffer, sizeof(CBPEROBJECT_PS));
		//list->CmdSetPSConstantBuffer(1, 1, &m_cPSBufferPerObject);
		//list->CmdSetPSTextureResource(0, (it->m_pDiffuseMap) ? it->m_pDiffuseMap->GetTexture() : m_pDefTexture);
		//list->CmdSetPSTextureResource(1, (it->m_pNormalMap) ? it->m_pNormalMap->GetTexture() : m_pDefNormalTexture);
		//list->CmdSetPSTextureResource(2, (it->m_pRoughnessSpecularMetallicCavityMap) ?
		//	it->m_pRoughnessSpecularMetallicCavityMap->GetTexture() : m_pDefTexture);
		//list->CmdSetPSTextureResource(3, (it->m_pMatMaskMap) ? it->m_pMatMaskMap->GetTexture() : m_pDefTexture);
		//list->CmdSetPSTextureResource(4, (it->m_pFlakesMap) ? it->m_pFlakesMap->GetTexture() : m_pDefFlakeTexture);

		list->CmdSetVertexBuffer(0, 1, it->m_pModel->GetWorld()->m_pVertexBuffer);

		for (auto objIt = it->m_pModel->GetWorld()->m_vObject.begin(); objIt != it->m_pModel->GetWorld()->m_vObject.end(); objIt++)
		{
			for (auto grpIt = objIt->second->m_vGroups.begin(); grpIt != objIt->second->m_vGroups.end(); grpIt++)
			{

				for (auto matIt = grpIt->second->m_vMaterials.begin(); matIt != grpIt->second->m_vMaterials.end(); matIt++)
				{
					if (matIt->second->m_pIndexBuffer == NULL || matIt->second->m_pMaterial == NULL)
						continue;
					list->CmdSetConstantBufferData(&m_cPSBufferPerObject, matIt->second->m_pMaterial, sizeof(CBPEROBJECT_PS));
					list->CmdSetPSConstantBuffer(1, 1, &m_cPSBufferPerObject);
					list->CmdSetPSTextureResource(0, (matIt->second->m_pMaterial->m_sDiffuseMap) 
						? matIt->second->m_pMaterial->m_sDiffuseMap->GetTexture() : m_pDefTexture);
					list->CmdSetPSTextureResource(1, (matIt->second->m_pMaterial->m_sNormalMap) 
						? matIt->second->m_pMaterial->m_sNormalMap->GetTexture() : m_pDefNormalTexture);
					list->CmdSetPSTextureResource(2, (matIt->second->m_pMaterial->m_sRSMCMap) 
						? matIt->second->m_pMaterial->m_sRSMCMap->GetTexture() : m_pDefTexture);
					list->CmdSetPSTextureResource(3, (matIt->second->m_pMaterial->m_sMatMaskMap) 
						? matIt->second->m_pMaterial->m_sMatMaskMap->GetTexture() : m_pDefTexture);
					list->CmdSetPSTextureResource(4, (matIt->second->m_pMaterial->m_sFlakesMap) 
						? matIt->second->m_pMaterial->m_sFlakesMap->GetTexture() : m_pDefFlakeTexture);
					list->CmdSetIndexBuffer(matIt->second->m_pIndexBuffer);
					list->CmdDrawIndexed(matIt->second->m_uIndexCount, 0, 0);
				}
			}
		}
	}

	list->CmdSetPSTextureResource(0, NULL);
	list->CmdSetPSTextureResource(1, NULL);
	list->CmdSetPSTextureResource(2, NULL);
	list->CmdSetPSTextureResource(3, NULL);
	list->CmdSetPSTextureResource(4, NULL);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);

	NTexture2DRes* renderTarget[1] = { 0 };
	list->CmdSetRendertargets(1, renderTarget, 0);


	for (auto it = m_vLights.begin(); it != m_vLights.end(); it++)
	{
		(*it)->VApplyToQueue(list, this);
	}

	for (auto it = m_vPostProcesses.begin(); it != m_vPostProcesses.end(); it++)
	{
		(*it)->VUpdateBuffer(this);
		(*it)->VApplyToQueue(list, this);
	}
}


void XM_CALLCONV PBRNDX11Effect::VSetWorld(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matWorld, value);
}

void XM_CALLCONV PBRNDX11Effect::VSetView(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matView, value);
}

void XM_CALLCONV PBRNDX11Effect::VSetProjection(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matProjection, value);
}

void PBRNDX11Effect::BuildBuffers(NGraphicsDevice* dev)
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	m_u32BoxVertexOffset = 0;
	m_u32GridVertexOffset = box.Vertices.size();
	m_u32SphereVertexOffset = m_u32GridVertexOffset + grid.Vertices.size();
	m_u32CylinderVertexOffset = m_u32SphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	m_u32BoxIndexCount = box.Indices.size();
	m_u32GridIndexCount = grid.Indices.size();
	m_u32SphereIndexCount = sphere.Indices.size();
	m_u32CylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	m_u32BoxIndexOffset = 0;
	m_u32GridIndexOffset = m_u32BoxIndexCount;
	m_u32SphereIndexOffset = m_u32GridIndexOffset + m_u32GridIndexCount;
	m_u32CylinderIndexOffset = m_u32SphereIndexOffset + m_u32SphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		m_u32BoxIndexCount +
		m_u32GridIndexCount +
		m_u32SphereIndexCount +
		m_u32CylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = *(DirectX::XMFLOAT3*)&box.Vertices[i].Position;
		vertices[k].Normal = *(DirectX::XMFLOAT3*)&box.Vertices[i].Normal;
		vertices[k].Tex = *(DirectX::XMFLOAT2*)&box.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(
			box.Vertices[i].TangentU.x,
			box.Vertices[i].TangentU.y,
			box.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = *(DirectX::XMFLOAT3*)&grid.Vertices[i].Position;
		vertices[k].Normal = *(DirectX::XMFLOAT3*)&grid.Vertices[i].Normal;
		vertices[k].Tex = *(DirectX::XMFLOAT2*)&grid.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(
			grid.Vertices[i].TangentU.x,
			grid.Vertices[i].TangentU.y,
			grid.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = *(DirectX::XMFLOAT3*)&sphere.Vertices[i].Position;
		vertices[k].Normal = *(DirectX::XMFLOAT3*)&sphere.Vertices[i].Normal;
		vertices[k].Tex = *(DirectX::XMFLOAT2*)&sphere.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(
			sphere.Vertices[i].TangentU.x,
			sphere.Vertices[i].TangentU.y,
			sphere.Vertices[i].TangentU.z,
			1.0f);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = *(DirectX::XMFLOAT3*)&cylinder.Vertices[i].Position;
		vertices[k].Normal = *(DirectX::XMFLOAT3*)&cylinder.Vertices[i].Normal;
		vertices[k].Tex = *(DirectX::XMFLOAT2*)&cylinder.Vertices[i].TexC;
		vertices[k].TangentU = DirectX::XMFLOAT4(
			cylinder.Vertices[i].TangentU.x,
			cylinder.Vertices[i].TangentU.y,
			cylinder.Vertices[i].TangentU.z,
			1.0f);
	}

	dev->CreateVertexBuffer(m_defVertBuff, N_B_USAGE_IMMUTABLE, sizeof(PosNormalTexTan)
		, 0, 0, &vertices[0], vertices.size(), sizeof(PosNormalTexTan), 0);

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	dev->CreateIndexBuffer(m_defIndBuff, N_B_USAGE_IMMUTABLE, sizeof(UINT)
		, 0, 0, &indices[0], totalIndexCount, N_R_FORMAT_R32_UINT, 0);

	dev->CreateConstantBuffer(m_cBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CBPEROBJECT_VS), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cPSBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CBPEROBJECT_PS), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cPSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(CBPERFRAME_PS), 1, D3D11_CPU_ACCESS_WRITE, 0);

	m_pDefTexture = new NTexture2DRes();

	N_G_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//float initData = 1.0f;
	UINT8 initData[4] = {255, 255, 255, 255};
	dev->CreateTextureFromDesc(*m_pDefTexture, texDesc, (UINTPTR)&initData, sizeof(UINT8) * 4);
	dev->CreateSRV(*m_pDefTexture);
	m_pDefTexture->ReleaseTexture();

	m_pDefNormalTexture = new NTexture2DRes();

	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//float initData = 1.0f;
	UINT8 initData3[4] = { 123, 123, 255, 255 };
	dev->CreateTextureFromDesc(*m_pDefNormalTexture, texDesc, (UINTPTR)&initData3, sizeof(UINT8) * 4);
	dev->CreateSRV(*m_pDefNormalTexture);
	m_pDefNormalTexture->ReleaseTexture();

	m_pDefFlakeTexture = new NTexture2DRes();

	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//float initData = 1.0f;
	UINT16 initData2[4] = { 0, 0, 0, 0 };
	dev->CreateTextureFromDesc(*m_pDefFlakeTexture, texDesc, (UINTPTR)&initData2, sizeof(UINT16) * 4);
	dev->CreateSRV(*m_pDefFlakeTexture);
	m_pDefFlakeTexture->ReleaseTexture();
}

void PBRNDX11Effect::UpdateBuffers()
{
	for (auto it = m_sRenderItems.begin(); it != m_sRenderItems.end(); it++)
	{
		UpdateBuffer(*it);
	}
	for (auto it = m_sModelRenderItems.begin(); it != m_sModelRenderItems.end(); it++)
	{
		UpdateBuffer(*it);
	}
}

void PBRNDX11Effect::UpdateBuffer(RENDERITEMBASE& buffer)
{
	DirectX::XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&buffer.m_matWorld);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&m_matView);
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&m_matProjection);
	DirectX::XMMATRIX wv = world * view;
	DirectX::XMMATRIX wvp = wv * proj;
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorld, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&buffer.m_matWorld)));
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorldView, DirectX::XMMatrixTranspose(wv));
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorldViewProj, DirectX::XMMatrixTranspose(wvp));
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorldViewProjTex
		, DirectX::XMMatrixTranspose(wvp * toTexSpace));

	DirectX::XMMATRIX A = DirectX::XMLoadFloat4x4(&buffer.m_matWorld);
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
	A = XMMatrixInverse(&det, A);
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matInvTranspose, A);
}

void PBRNDX11Effect::BuildGBuffers(NGraphicsDevice& dev)
{
	m_pGBuffers[GBUFFERS_DEPTHSTENCIL] = new NTexture2DRes();

	N_G_TEXTURE2D_DESC texDesc;
	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_DEPTHSTENCIL], texDesc, NULL, 0);

	N_G_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dev.CreateDSV(*m_pGBuffers[GBUFFERS_DEPTHSTENCIL], dsvDesc);

	N_G_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	dev.CreateSRV(*m_pGBuffers[GBUFFERS_DEPTHSTENCIL], srvDesc);

	m_pGBuffers[GBUFFERS_DEPTHSTENCIL]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS]);

	m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_DIFFUSEALBEDO] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_DIFFUSEALBEDO], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_DIFFUSEALBEDO]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_DIFFUSEALBEDO]);

	m_pGBuffers[GBUFFERS_DIFFUSEALBEDO]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK]);

	m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_CUSTOMVARS] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_CUSTOMVARS], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_CUSTOMVARS]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_CUSTOMVARS]);

	m_pGBuffers[GBUFFERS_CUSTOMVARS]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_CUSTOMUINT4] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_CUSTOMUINT4], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_CUSTOMUINT4]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_CUSTOMUINT4]);

	m_pGBuffers[GBUFFERS_CUSTOMUINT4]->ReleaseTexture();

	// =====================================================

	m_pGBuffers[GBUFFERS_CUSTOMUINT42] = new NTexture2DRes();

	texDesc.Width = dev.GetClientWidth();
	texDesc.Height = dev.GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev.CreateTextureFromDesc(*m_pGBuffers[GBUFFERS_CUSTOMUINT42], texDesc, NULL, 0);

	dev.CreateSRV(*m_pGBuffers[GBUFFERS_CUSTOMUINT42]);
	dev.CreateRTV(*m_pGBuffers[GBUFFERS_CUSTOMUINT42]);

	m_pGBuffers[GBUFFERS_CUSTOMUINT42]->ReleaseTexture();

	// =====================================================

	for (UINT32 i = 0; i < LIGHTACCUM_NO; i++)
	{
		m_pLightAccumBuffers[i] = new NTexture2DRes();

		texDesc.Width = dev.GetClientWidth();
		texDesc.Height = dev.GetClientHeight();
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		dev.CreateTextureFromDesc(*m_pLightAccumBuffers[i], texDesc, NULL, 0);

		dev.CreateSRV(*m_pLightAccumBuffers[i]);
		dev.CreateRTV(*m_pLightAccumBuffers[i]);

		m_pLightAccumBuffers[i]->ReleaseTexture();
	}
}

void PBRNDX11Effect::ClearGBuffers(NRenderCmdList& list)
{
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS], m_pGBuffers[GBUFFERS_DEPTHSTENCIL], 1.f, 0, 0.f, 0.f, 0.f);
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_DIFFUSEALBEDO], nullptr, 1.f, 0, 0.f, 0.f, 0.f);
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK], nullptr, 1.f, 0, 0.f, 0.f, 0.f);
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_CUSTOMVARS], nullptr, 1.f, 0, 0.f, 0.f, 0.f);
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_CUSTOMUINT4], nullptr, 1.f, 0, 0.f, 0.f, 0.f);
	list.CmdClear(N_B_ALL, m_pGBuffers[GBUFFERS_CUSTOMUINT42], nullptr, 1.f, 0, 0.f, 0.f, 0.f);

	for (UINT32 i = 0; i < LIGHTACCUM_NO; i++)
	{
		list.CmdClear(N_B_ALL, m_pLightAccumBuffers[i], nullptr, 1.f, 0, 0.f, 0.f, 0.f);
	}
}

void PBRNDX11Effect::SetGBuffers(NRenderCmdList& list)
{
	list.CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list.CmdSetPSTextureResource(0, m_pGBuffers[GBUFFERS_NORMALDEPTHROUGHNESS]);
	list.CmdSetPSTextureResource(1, m_pGBuffers[GBUFFERS_DIFFUSEALBEDO]);
	list.CmdSetPSTextureResource(2, m_pGBuffers[GBUFFERS_SPECULARCAVITYMETALLICMATMASK]);
	list.CmdSetPSTextureResource(3, m_pGBuffers[GBUFFERS_CUSTOMVARS]);
	list.CmdSetPSTextureResource(4, m_pGBuffers[GBUFFERS_CUSTOMUINT4]);
	list.CmdSetPSTextureResource(5, m_pGBuffers[GBUFFERS_CUSTOMUINT42]);
}

void PBRNDX11Effect::UnsetGBuffers(NRenderCmdList& list)
{
	list.CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list.CmdSetPSTextureResource(0, NULL);
	list.CmdSetPSTextureResource(1, NULL);
	list.CmdSetPSTextureResource(2, NULL);
	list.CmdSetPSTextureResource(3, NULL);
	list.CmdSetPSTextureResource(4, NULL);
	list.CmdSetPSTextureResource(5, NULL);
}

BOOL PBRNDX11Effect::DeferredDirLight::g_bIsGlobalBufferInit = false;
std::shared_ptr<NResHandle> PBRNDX11Effect::DeferredDirLight::g_pVS;
std::shared_ptr<NResHandle> PBRNDX11Effect::DeferredDirLight::g_pPS;
std::shared_ptr<NResHandle> PBRNDX11Effect::DeferredDirLight::g_pShadowVS;
std::shared_ptr<NResHandle> PBRNDX11Effect::DeferredDirLight::g_pShadowPS;
NConstantBuffer PBRNDX11Effect::DeferredDirLight::g_cVSBufferPerFrame;
NConstantBuffer PBRNDX11Effect::DeferredDirLight::g_cPSBufferPerFrame;
NConstantBuffer PBRNDX11Effect::DeferredDirLight::g_cShadowBufferPerFrame;
NConstantBuffer PBRNDX11Effect::DeferredDirLight::g_cShadowBufferPerObject;
NVertexBuffer PBRNDX11Effect::DeferredDirLight::g_screenVertBuff;
NIndexBuffer PBRNDX11Effect::DeferredDirLight::g_screenIndBuff;


PBRNDX11Effect::DeferredDirLight::DeferredDirLight(DirectX::XMFLOAT3& dir, DirectX::XMFLOAT3& color, BOOL shadowed)
	: m_bIsShadowed(shadowed)
	, m_bIsNeedUpdateBuffer(true)
	, m_bIsBuildShadowBuffer(false)
	, m_bIsNeedUpdateShadowBuffer(true)
{
	m_sLightBufferPerFrame.m_f3Dir = dir;
	m_sLightBufferPerFrame.m_f3Color = color;
}

void PBRNDX11Effect::DeferredDirLight::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	if (!g_bIsGlobalBufferInit)
	{
		g_bIsGlobalBufferInit = true;

		N_G_SHADER_LAYOUT simpleLayout;
		simpleLayout.numElements = 2;
		simpleLayout.layouts[0].SemanticName = "POSITION";
		simpleLayout.layouts[0].SemanticIndex = 0;
		simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		simpleLayout.layouts[0].InputSlot = 0;
		simpleLayout.layouts[0].AlignedByteOffset = 0;
		simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		simpleLayout.layouts[0].InstanceDataStepRate = 0;

		simpleLayout.layouts[1].SemanticName = "TEXCOORD";
		simpleLayout.layouts[1].SemanticIndex = 0;
		simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		simpleLayout.layouts[1].InputSlot = 0;
		simpleLayout.layouts[1].AlignedByteOffset = 12;
		simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		simpleLayout.layouts[1].InstanceDataStepRate = 0;

		N_G_SHADER_LAYOUT layout;
		layout.numElements = 4;
		layout.layouts[0].SemanticName = "POSITION";
		layout.layouts[0].SemanticIndex = 0;
		layout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		layout.layouts[0].InputSlot = 0;
		layout.layouts[0].AlignedByteOffset = 0;
		layout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout.layouts[0].InstanceDataStepRate = 0;

		layout.layouts[1].SemanticName = "NORMAL";
		layout.layouts[1].SemanticIndex = 0;
		layout.layouts[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		layout.layouts[1].InputSlot = 0;
		layout.layouts[1].AlignedByteOffset = 12;
		layout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout.layouts[1].InstanceDataStepRate = 0;

		layout.layouts[2].SemanticName = "TEXCOORD";
		layout.layouts[2].SemanticIndex = 0;
		layout.layouts[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		layout.layouts[2].InputSlot = 0;
		layout.layouts[2].AlignedByteOffset = 24;
		layout.layouts[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout.layouts[2].InstanceDataStepRate = 0;

		layout.layouts[3].SemanticName = "TANGENT";
		layout.layouts[3].SemanticIndex = 0;
		layout.layouts[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		layout.layouts[3].InputSlot = 0;
		layout.layouts[3].AlignedByteOffset = 32;
		layout.layouts[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout.layouts[3].InstanceDataStepRate = 0;


		NRes vsFile("FX\\DeferredBasicDirLight_vs.cso");
		g_pVS = resCache->GetHandle(&vsFile);
		((NVertexShaderNResExtraData*)g_pVS->GetExtra().get())->Initialize(simpleLayout);

		NRes psFile("FX\\DeferredBasicDirLight_ps.cso");
		g_pPS = resCache->GetHandle(&psFile);

		NRes vsShadowFile("FX\\DeferredBasicDepthOnly_vs.cso");
		g_pShadowVS = resCache->GetHandle(&vsShadowFile);
		((NVertexShaderNResExtraData*)g_pShadowVS->GetExtra().get())->Initialize(layout);

		NRes psShadowFile("FX\\DeferredBasicDepthOnly_ps.cso");
		g_pShadowPS = resCache->GetHandle(&psShadowFile);

		GeometryGenerator geoGen;
		GeometryGenerator::MeshData quad;
		geoGen.CreateFullscreenQuad(quad);
		std::vector<PosTex> screenVertices(quad.Vertices.size());
		for (UINT32 i = 0; i < quad.Vertices.size(); i++)
		{
			screenVertices[i].Pos = *(DirectX::XMFLOAT3*)&quad.Vertices[i].Position;
			screenVertices[i].Tex = *(DirectX::XMFLOAT2*)&quad.Vertices[i].TexC;
		}

		dev->CreateVertexBuffer(g_screenVertBuff, N_B_USAGE_IMMUTABLE, sizeof(PosTex)
			, 0, 0, &screenVertices[0], screenVertices.size(), sizeof(PosTex), 0);
		dev->CreateIndexBuffer(g_screenIndBuff, N_B_USAGE_IMMUTABLE, sizeof(UINT)
			, 0, 0, &quad.Indices[0], quad.Indices.size(), N_R_FORMAT_R32_UINT, 0);

		dev->CreateConstantBuffer(g_cShadowBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(SHADOW_PERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
		dev->CreateConstantBuffer(g_cShadowBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(SHADOW_PEROBJECT), 1, D3D11_CPU_ACCESS_WRITE, 0);
		dev->CreateConstantBuffer(g_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(DIRLIGHT_VS_PERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
		dev->CreateConstantBuffer(g_cPSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(DIRLIGHT_PERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	}

	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSLightBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSLightBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	if (m_bIsShadowed)
	{
		BuildShadowBuffer(dev);
	}
}

PBRNDX11Effect::DeferredDirLight::~DeferredDirLight()
{
	N_DELETE(m_pShadowMap);
}

void PBRNDX11Effect::DeferredDirLight::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	if (m_bIsNeedUpdateShadowBuffer)
		UpdateShadowBuffer();

	list->CmdSetViewports(1, &m_sShadowViewport);
	NTexture2DRes* renderTarget[1] = { 0 };
	list->CmdSetRendertargets(1, renderTarget, m_pShadowMap);
	list->CmdClear(N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER, nullptr, m_pShadowMap, 1.f);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_depthRS);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)g_pShadowVS->GetExtra().get())->GetShader());
	list->CmdSetVertexBuffer(0, 1, &effect->m_defVertBuff);
	list->CmdSetIndexBuffer(&effect->m_defIndBuff);
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)g_pShadowPS->GetExtra().get())->GetShader());
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetConstantBufferData(&g_cShadowBufferPerFrame, &m_sShadowBufferPerFrame, sizeof(SHADOW_PERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &g_cShadowBufferPerFrame);
	for (auto it = effect->m_sRenderItems.begin(); it != effect->m_sRenderItems.end(); it++)
	{
		list->CmdSetConstantBufferData(&g_cShadowBufferPerObject, &it->m_buffer, sizeof(SHADOW_PEROBJECT));
		list->CmdSetVSConstantBuffer(1, 1, &g_cShadowBufferPerObject);
		list->CmdSetPSTextureResource(0, (it->m_pDiffuseMap) ? it->m_pDiffuseMap->GetTexture() : effect->m_pDefTexture);
		list->CmdDrawIndexed(it->m_uIndexCount, it->m_uIndexOffset, it->m_uVertexOffset);
	}

	for (auto it = effect->m_sModelRenderItems.begin(); it != effect->m_sModelRenderItems.end(); it++)
	{
		if (it->m_pModel->GetWorld() == NULL)
			continue;

		list->CmdSetConstantBufferData(&g_cShadowBufferPerObject, &it->m_buffer, sizeof(SHADOW_PEROBJECT));
		list->CmdSetVSConstantBuffer(1, 1, &g_cShadowBufferPerObject);
		//list->CmdSetPSTextureResource(0, (it->m_pDiffuseMap) ? it->m_pDiffuseMap->GetTexture() : effect->m_pDefTexture);

		list->CmdSetVertexBuffer(0, 1, it->m_pModel->GetWorld()->m_pVertexBuffer);

		for (auto objIt = it->m_pModel->GetWorld()->m_vObject.begin(); objIt != it->m_pModel->GetWorld()->m_vObject.end(); objIt++)
		{
			for (auto grpIt = objIt->second->m_vGroups.begin(); grpIt != objIt->second->m_vGroups.end(); grpIt++)
			{
				for (auto matIt = grpIt->second->m_vMaterials.begin(); matIt != grpIt->second->m_vMaterials.end(); matIt++)
				{
					if (matIt->second->m_pIndexBuffer == NULL || matIt->second->m_pMaterial == NULL)
						continue;

					list->CmdSetPSTextureResource(0, (matIt->second->m_pMaterial->m_sDiffuseMap) ? matIt->second->m_pMaterial->m_sDiffuseMap->GetTexture() : effect->m_pDefTexture);
					list->CmdSetIndexBuffer(matIt->second->m_pIndexBuffer);
					list->CmdDrawIndexed(matIt->second->m_uIndexCount, 0, 0);
				}
			}
		}
	}

	if (m_bIsNeedUpdateBuffer)
		UpdateBuffer(effect);

	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);
	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_postProcessDSS, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	list->CmdSetBlendState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_deferredLightAccumBS, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)g_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)g_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	effect->SetGBuffers(*list);
	list->CmdSetConstantBufferData(&g_cVSBufferPerFrame, &m_sVSLightBufferPerFrame, sizeof(DIRLIGHT_VS_PERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &g_cVSBufferPerFrame);
	list->CmdSetConstantBufferData(&g_cPSBufferPerFrame, &m_sLightBufferPerFrame, sizeof(DIRLIGHT_PERFRAME));
	list->CmdSetPSConstantBuffer(0, 1, &g_cPSBufferPerFrame);
	if (m_bIsShadowed)
	{
		list->CmdSetPSTextureResource(6, m_pShadowMap);
		list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_shadowSampler, 1, 1);
	}

	list->CmdDrawIndexed(6, 0, 0);

	list->CmdSetPSTextureResource(6, NULL);
	effect->UnsetGBuffers(*list);
	list->CmdSetRendertargets(1, renderTarget, NULL);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::DeferredDirLight::VSetLightDir(DirectX::FXMVECTOR& dir)
{
	DirectX::XMStoreFloat3(&m_sLightBufferPerFrame.m_f3Dir, dir);
	m_bIsNeedUpdateBuffer = true;
	m_bIsNeedUpdateShadowBuffer = true;
}

void PBRNDX11Effect::DeferredDirLight::VSetLightColor(DirectX::FXMVECTOR& color)
{
	DirectX::XMStoreFloat3(&m_sLightBufferPerFrame.m_f3Color, color);
	m_bIsNeedUpdateBuffer = true;
}

void PBRNDX11Effect::DeferredDirLight::VSetShadowed(BOOL shadowed)
{
	m_bIsShadowed = shadowed;
}

void PBRNDX11Effect::DeferredDirLight::BuildShadowBuffer(NGraphicsDevice* dev)
{
	m_sSceneBound.m_vCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_sSceneBound.m_fRadius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	m_sShadowViewport.TopLeftX = 0;
	m_sShadowViewport.TopLeftY = 0;
	m_sShadowViewport.Width = 2048.f;
	m_sShadowViewport.Height = 2048.f;
	m_sShadowViewport.MinDepth = 0.f;
	m_sShadowViewport.MaxDepth = 1.f;

	m_pShadowMap = new NTexture2DRes();

	N_G_TEXTURE2D_DESC texDesc;
	texDesc.Width = 2048;
	texDesc.Height = 2048;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev->CreateTextureFromDesc(*m_pShadowMap, texDesc, NULL, 0);

	N_G_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dev->CreateDSV(*m_pShadowMap, dsvDesc);

	N_G_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	dev->CreateSRV(*m_pShadowMap, srvDesc);

	m_pShadowMap->ReleaseTexture();

	UpdateShadowBuffer();

	m_bIsBuildShadowBuffer = true;
}

void PBRNDX11Effect::DeferredDirLight::UpdateShadowBuffer()
{
	DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&m_sLightBufferPerFrame.m_f3Dir);
	DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&m_sSceneBound.m_vCenter);
	DirectX::XMVECTOR lightPos = -2.0f * m_sSceneBound.m_fRadius * lightDir;
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(lightPos, targetPos, up);
	DirectX::XMFLOAT3 sphereCenterLS;
	DirectX::XMStoreFloat3(&sphereCenterLS, DirectX::XMVector3TransformCoord(targetPos, view));
	float l = sphereCenterLS.x - m_sSceneBound.m_fRadius;
	float r = sphereCenterLS.x + m_sSceneBound.m_fRadius;
	float t = sphereCenterLS.y + m_sSceneBound.m_fRadius;
	float b = sphereCenterLS.y - m_sSceneBound.m_fRadius;
	float n = sphereCenterLS.z - m_sSceneBound.m_fRadius;
	float f = sphereCenterLS.z + m_sSceneBound.m_fRadius;
	DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	DirectX::XMMATRIX ndc2Tex(0.5f, 0.f, 0.f, 0.f,
		0.f, -0.5f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.5f, 0.5f, 0.f, 1.f);
	DirectX::XMMATRIX shadowTransform = view * proj * ndc2Tex;

	DirectX::XMStoreFloat4x4(&m_sShadowBufferPerFrame.m_matLightViewProj, DirectX::XMMatrixTranspose(view * proj));
	DirectX::XMStoreFloat4x4(&m_sLightBufferPerFrame.m_matLightShadowTransform, DirectX::XMMatrixTranspose(shadowTransform));

	//m_bIsNeedUpdateShadowBuffer = false;
}

void PBRNDX11Effect::DeferredDirLight::UpdateBuffer(PBRNDX11Effect* effect)
{
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);
	m_sLightBufferPerFrame.m_f3ViewPos = effect->m_f3CamPos;
	//view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-m_sLightBufferPerFrame.m_f3ViewPos.x
		, -m_sLightBufferPerFrame.m_f3ViewPos.y, -m_sLightBufferPerFrame.m_f3ViewPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = translatedView * proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSLightBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));

	DirectX::XMStoreFloat4x4(&m_sLightBufferPerFrame.m_matProjection, DirectX::XMMatrixTranspose(proj));
	m_sLightBufferPerFrame.m_fNearZ = effect->m_sPSBufferPerFrame.m_fNearZ;
	m_sLightBufferPerFrame.m_fFarZ = effect->m_sPSBufferPerFrame.m_fFarZ;
	//m_bIsNeedUpdateBuffer = false;
}

BOOL PBRNDX11Effect::BasicPostProcessEffect::g_bIsGlobalBufferInit = false;
NVertexBuffer PBRNDX11Effect::BasicPostProcessEffect::g_screenVertBuff;
NIndexBuffer PBRNDX11Effect::BasicPostProcessEffect::g_screenIndBuff;

PBRNDX11Effect::BasicPostProcessEffect::BasicPostProcessEffect()
{

}

void PBRNDX11Effect::BasicPostProcessEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	if (!g_bIsGlobalBufferInit)
	{
		g_bIsGlobalBufferInit = true; 
		
		GeometryGenerator geoGen;
		GeometryGenerator::MeshData quad;
		geoGen.CreateFullscreenQuad(quad);
		std::vector<PosTex> screenVertices(quad.Vertices.size());
		for (UINT32 i = 0; i < quad.Vertices.size(); i++)
		{
			screenVertices[i].Pos = *(DirectX::XMFLOAT3*)&quad.Vertices[i].Position;
			screenVertices[i].Tex = *(DirectX::XMFLOAT2*)&quad.Vertices[i].TexC;
		}

		dev->CreateVertexBuffer(g_screenVertBuff, N_B_USAGE_IMMUTABLE, sizeof(PosTex)
			, 0, 0, &screenVertices[0], screenVertices.size(), sizeof(PosTex), 0);
		dev->CreateIndexBuffer(g_screenIndBuff, N_B_USAGE_IMMUTABLE, sizeof(UINT)
			, 0, 0, &quad.Indices[0], quad.Indices.size(), N_R_FORMAT_R32_UINT, 0);
	}
}


PBRNDX11Effect::AmbientBRDFEffect::AmbientBRDFEffect()
	:BasicPostProcessEffect()
{

}

void PBRNDX11Effect::AmbientBRDFEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\AmbientBRDF_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\AmbientBRDF_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	NRes brdfFile("Textures\\brdflutmap.dds");
	NRes prefilterFile("Textures\\prefilteredmap_using.dds");

	m_pBrdfLutMap = resCache->GetHandle(&brdfFile);
	m_pAmbientCubeMap = resCache->GetHandle(&prefilterFile);

	N_G_TEXTURE2D_DESC pfMapDesc;
	((NDDSTextureNResExtraData*)m_pAmbientCubeMap->GetExtra().get())->GetTexture()->GetDesc(pfMapDesc);


	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);
	m_sPSBufferPerAmbientCubeMap.m_ambientCubeMapColor.x = m_sPSBufferPerAmbientCubeMap.m_ambientCubeMapColor.y = 
		m_sPSBufferPerAmbientCubeMap.m_ambientCubeMapColor.z = 
		m_sPSBufferPerAmbientCubeMap.m_ambientCubeMapColor.w = 1.0f;
	m_sPSBufferPerAmbientCubeMap.m_ambientCubeMipAdjust.x = 1.0f;
	m_sPSBufferPerAmbientCubeMap.m_ambientCubeMipAdjust.y = 0.0f;
	m_sPSBufferPerAmbientCubeMap.m_ambientCubeMipAdjust.z = 3.0f;
	m_sPSBufferPerAmbientCubeMap.m_ambientCubeMipAdjust.w = pfMapDesc.MipLevels-1;

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cPSBufferPerAmbientCubeMap, N_B_USAGE_DYNAMIC, sizeof(PSBUFFERPERAMBIENTCUBEMAP), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

void PBRNDX11Effect::AmbientBRDFEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);
	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_postProcessDSS, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_deferredLightAccumBS, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	effect->SetGBuffers(*list);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 1, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetConstantBufferData(&m_cPSBufferPerAmbientCubeMap, &m_sPSBufferPerAmbientCubeMap, sizeof(PSBUFFERPERAMBIENTCUBEMAP));
	list->CmdSetPSConstantBuffer(0, 1, &m_cPSBufferPerAmbientCubeMap);
	list->CmdSetPSTextureResource(6, ((NDDSTextureNResExtraData*)m_pBrdfLutMap->GetExtra().get())->GetTexture());
	list->CmdSetPSTextureResource(7, ((NDDSTextureNResExtraData*)m_pAmbientCubeMap->GetExtra().get())->GetTexture());

	list->CmdDrawIndexed(6, 0, 0);

	list->CmdSetPSTextureResource(7, NULL);
	list->CmdSetPSTextureResource(6, NULL);
	effect->UnsetGBuffers(*list);
	NTexture2DRes* defRenderTarget2[1] = { NULL };
	list->CmdSetRendertargets(1, defRenderTarget2, 0);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::AmbientBRDFEffect::VUpdateBuffer(PBRNDX11Effect* effect)
{
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-effect->m_f3CamPos.x
		, -effect->m_f3CamPos.y, -effect->m_f3CamPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = translatedView * proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));
}


PBRNDX11Effect::CopyToBackBufferEffect::CopyToBackBufferEffect()
	:BasicPostProcessEffect()
{

}

void PBRNDX11Effect::CopyToBackBufferEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasicPostProc_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\CopyRenderTarget_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);


	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

void PBRNDX11Effect::CopyToBackBufferEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { NGraphicsDevice::g_pMainGraphicsDevice->GetMainRenderTargets() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);
	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_postProcessDSS, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetPSTextureResource(0, effect->GetCurrentAccumBuffer());

	list->CmdDrawIndexed(6, 0, 0);


	list->CmdSetPSTextureResource(0, NULL);
	NTexture2DRes* defRenderTarget2[1] = { 0 };
	list->CmdSetRendertargets(1, defRenderTarget2, 0);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

PBRNDX11Effect::BasicSkyEffect::BasicSkyEffect(std::shared_ptr<NResHandle> cubeMap)
	:BasicPostProcessEffect()
	, m_pCubeMap(cubeMap)
{

}

void PBRNDX11Effect::BasicSkyEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasicSky_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\DeferredBasicSky_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

void PBRNDX11Effect::BasicSkyEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, NGraphicsDevice::g_pMainGraphicsDevice->GetMainDepthStencil());
	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_lessEqualsDSS, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_deferredLightAccumBS, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetPSTextureResource(0, ((NDDSTextureNResExtraData*)m_pCubeMap->GetExtra().get())->GetTexture());

	list->CmdDrawIndexed(6, 0, 0);

	list->CmdSetPSTextureResource(0, NULL);
	NTexture2DRes* defRenderTarget2[1] = { NULL };
	list->CmdSetRendertargets(1, defRenderTarget2, NULL);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::BasicSkyEffect::VUpdateBuffer(PBRNDX11Effect* effect)
{
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-effect->m_f3CamPos.x
		, -effect->m_f3CamPos.y, -effect->m_f3CamPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = translatedView * proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));
}


PBRNDX11Effect::GlitterEffect::GlitterEffect()
	:BasicPostProcessEffect()
{
	m_sPSBufferPerFrame.m_fMaskMat = 1.0f;
}

void PBRNDX11Effect::GlitterEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasicPostProc_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\GlitterPostProc_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	NRes normFile("Textures\\bricks_nmap.dds");
	m_pNormalMap = resCache->GetHandle(&normFile);
	NRes flakesFile("Textures\\floorflake.dds");
	m_pFlakesMap = resCache->GetHandle(&flakesFile);

	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cPSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(PSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

void PBRNDX11Effect::GlitterEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetDefViewports();
	NTexture2DRes* currentAcumMap = effect->GetCurrentAccumBuffer();
	effect->SwitchAccumBufferInd();
	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);
	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_postProcessDSS, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	effect->SetGBuffers(*list);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 1, 1);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_discreteSampler, 2, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetConstantBufferData(&m_cPSBufferPerFrame, &m_sPSBufferPerFrame, sizeof(PSBUFFERPERFRAME));
	list->CmdSetPSConstantBuffer(0, 1, &m_cPSBufferPerFrame);
	list->CmdSetPSTextureResource(4, currentAcumMap);
	list->CmdSetPSTextureResource(5, ((NDDSTextureNResExtraData*)m_pNormalMap->GetExtra().get())->GetTexture());
	list->CmdSetPSTextureResource(6, ((NDDSTextureNResExtraData*)m_pFlakesMap->GetExtra().get())->GetTexture());
	//list->CmdSetPSTextureResource(6, GenFlakesMapNDX11Effect::gMainEffect->GetMainResult());

	list->CmdDrawIndexed(6, 0, 0);

	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::GlitterEffect::VUpdateBuffer(PBRNDX11Effect* effect)
{
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-effect->m_f3CamPos.x
		, -effect->m_f3CamPos.y, -effect->m_f3CamPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = translatedView * proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));
}


PBRNDX11Effect::SSAOEffect::SSAOEffect()
	:BasicPostProcessEffect()
{
	m_pAmbientMap[0] = m_pAmbientMap[1] = NULL;
}
PBRNDX11Effect::SSAOEffect::~SSAOEffect()
{
	N_DELETE(m_pAmbientMap[0]);
	N_DELETE(m_pAmbientMap[1]);
}

void PBRNDX11Effect::SSAOEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasicPostProc_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\SSAOPostProc_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	NRes blurPsFile("FX\\BlurTex_ps.cso");
	m_pBlurPS = resCache->GetHandle(&blurPsFile);

	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	for (UINT32 i = 0; i < 2; i++)
	{
		m_pAmbientMap[i] = new NTexture2DRes();
		N_G_TEXTURE2D_DESC texDesc;
		texDesc.Width = dev->GetClientWidth() * 0.5f;
		texDesc.Height = dev->GetClientHeight() * 0.5f;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		dev->CreateTextureFromDesc(*m_pAmbientMap[i], texDesc, NULL, 0);
		dev->CreateSRV(*m_pAmbientMap[i]);
		dev->CreateRTV(*m_pAmbientMap[i]);
		m_pAmbientMap[i]->ReleaseTexture();
	}

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cPSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(PSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);

	m_sBlurPSBufferPerFrame[0].m_vOffsetSize.x = 2.f / dev->GetClientWidth();
	m_sBlurPSBufferPerFrame[0].m_vOffsetSize.y = 0.f;
	m_sBlurPSBufferPerFrame[1].m_vOffsetSize.x = 0.f;
	m_sBlurPSBufferPerFrame[1].m_vOffsetSize.y = 2.f / dev->GetClientHeight();
	dev->CreateConstantBuffer(m_cBlurPSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(BLURPSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);

	m_ssaoViewport.Width = dev->GetClientWidth() * 0.5f;
	m_ssaoViewport.Height = dev->GetClientHeight() * 0.5f;
	m_ssaoViewport.TopLeftX = 0.0f;
	m_ssaoViewport.TopLeftY = 0.0f;
	m_ssaoViewport.MinDepth = 0.0f;
	m_ssaoViewport.MaxDepth = 1.0f;
}

void PBRNDX11Effect::SSAOEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetViewports(1, &m_ssaoViewport);
	NTexture2DRes* currentAcumMap = effect->GetCurrentAccumBuffer();
	//effect->SwitchAccumBufferInd();
	//NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	NTexture2DRes* ssaoRenderTarget[1] = { m_pAmbientMap[0] };
	list->CmdSetRendertargets(1, ssaoRenderTarget, 0);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	effect->SetGBuffers(*list);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 1, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetConstantBufferData(&m_cPSBufferPerFrame, &m_sPSBufferPerFrame, sizeof(PSBUFFERPERFRAME));
	list->CmdSetPSConstantBuffer(0, 1, &m_cPSBufferPerFrame);
	list->CmdSetPSTextureResource(6, currentAcumMap);

	list->CmdDrawIndexed(6, 0, 0);

	// Blur
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pBlurPS->GetExtra().get())->GetShader());
	for (UINT32 i = 0; i < 2; i++)
	{
		NTexture2DRes* blurRenderTarget[1] = { m_pAmbientMap[!i] };
		list->CmdSetRendertargets(1, blurRenderTarget, 0);
		//if (i == 1)
		//{
		//	effect->SwitchAccumBufferInd();
		//	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
		//	list->CmdSetRendertargets(1, defRenderTarget, 0);
		//}
		list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_blurSampler, 1, 1);
		list->CmdSetConstantBufferData(&m_cBlurPSBufferPerFrame, &m_sBlurPSBufferPerFrame[i], sizeof(BLURPSBUFFERPERFRAME));
		list->CmdSetPSConstantBuffer(0, 1, &m_cBlurPSBufferPerFrame);
		list->CmdSetPSTextureResource(6, m_pAmbientMap[i]);

		list->CmdDrawIndexed(6, 0, 0);

		list->CmdSetPSTextureResource(6, NULL);
	}


	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::SSAOEffect::VUpdateBuffer(PBRNDX11Effect* effect)
{

	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-effect->m_f3CamPos.x
		, -effect->m_f3CamPos.y, -effect->m_f3CamPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = /*translatedView **/ proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));

	const DirectX::XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	DirectX::XMMATRIX viewToTex = DirectX::XMMatrixMultiply(proj, T);
	DirectX::XMStoreFloat4x4(&m_sPSBufferPerFrame.m_matViewToTexSpace, DirectX::XMMatrixTranspose(viewToTex));

	m_sPSBufferPerFrame.m_fFarZ = SkinnedMeshApp::Instance().GetCurrentCamera()->GetFarZ();
	m_sBlurPSBufferPerFrame[0].m_fFarZ = m_sBlurPSBufferPerFrame[1].m_fFarZ = SkinnedMeshApp::Instance().GetCurrentCamera()->GetFarZ();
}

PBRNDX11Effect::CompositeEffect::CompositeEffect(SSAOEffect* ssao)
	:BasicPostProcessEffect()
	, m_pSSAO(ssao)
{
}
PBRNDX11Effect::CompositeEffect::~CompositeEffect()
{
}

void PBRNDX11Effect::CompositeEffect::VBuildBuffer(NGraphicsDevice* dev, NResCache* resCache)
{
	BasicPostProcessEffect::VBuildBuffer(dev, resCache);

	N_G_SHADER_LAYOUT simpleLayout;
	simpleLayout.numElements = 2;
	simpleLayout.layouts[0].SemanticName = "POSITION";
	simpleLayout.layouts[0].SemanticIndex = 0;
	simpleLayout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	simpleLayout.layouts[0].InputSlot = 0;
	simpleLayout.layouts[0].AlignedByteOffset = 0;
	simpleLayout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[0].InstanceDataStepRate = 0;

	simpleLayout.layouts[1].SemanticName = "TEXCOORD";
	simpleLayout.layouts[1].SemanticIndex = 0;
	simpleLayout.layouts[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	simpleLayout.layouts[1].InputSlot = 0;
	simpleLayout.layouts[1].AlignedByteOffset = 12;
	simpleLayout.layouts[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	simpleLayout.layouts[1].InstanceDataStepRate = 0;

	NRes vsFile("FX\\DeferredBasicPostProc_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(simpleLayout);

	NRes psFile("FX\\CompositePostProc_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);


	DirectX::XMMATRIX vsWorldViewProj(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	vsWorldViewProj = DirectX::XMMatrixTranspose(vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matWorldViewProj, vsWorldViewProj);
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, vsWorldViewProj);

	dev->CreateConstantBuffer(m_cVSBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(VSBUFFERPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
}

void PBRNDX11Effect::CompositeEffect::VApplyToQueue(NRenderCmdList* list, PBRNDX11Effect* effect)
{
	BasicPostProcessEffect::VApplyToQueue(list, effect);

	list->CmdSetDefViewports();
	NTexture2DRes* currentAcumMap = effect->GetCurrentAccumBuffer();
	effect->SwitchAccumBufferInd();
	NTexture2DRes* defRenderTarget[1] = { effect->GetCurrentAccumBuffer() };
	list->CmdSetRendertargets(1, defRenderTarget, 0);

	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
	float lightBlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);

	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());

	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexBuffer(0, 1, &g_screenVertBuff);
	list->CmdSetIndexBuffer(&g_screenIndBuff);

	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 1, 1);
	list->CmdSetConstantBufferData(&m_cVSBufferPerFrame, &m_sVSBufferPerFrame, sizeof(VSBUFFERPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cVSBufferPerFrame);
	list->CmdSetPSTextureResource(0, currentAcumMap);
	list->CmdSetPSTextureResource(1, m_pSSAO->GetAmbientMap());

	list->CmdDrawIndexed(6, 0, 0);

	list->CmdSetPSTextureResource(0, NULL);
	list->CmdSetPSTextureResource(1, NULL);
	list->CmdSetBlendState(NULL, lightBlendFactor, 0xffffffff);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
}

void PBRNDX11Effect::CompositeEffect::VUpdateBuffer(PBRNDX11Effect* effect)
{

	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&effect->m_matProjection);
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&effect->m_matView);

	DirectX::XMMATRIX translatedView = DirectX::XMMatrixTranslation(-effect->m_f3CamPos.x
		, -effect->m_f3CamPos.y, -effect->m_f3CamPos.z) * view;
	DirectX::XMMATRIX translatedViewProj = /*translatedView **/ proj;
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(translatedViewProj);
	DirectX::XMMATRIX invTranslatedViewProj = XMMatrixInverse(&det, translatedViewProj);
	DirectX::XMMATRIX screenToTranslatedWorld = DirectX::XMMatrixSet(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, effect->m_matProjection._33, 1.f,
		0.f, 0.f, effect->m_matProjection._43, 0.f) * invTranslatedViewProj;
	DirectX::XMStoreFloat4x4(&m_sVSBufferPerFrame.m_matScreenToTranslatedWorld, DirectX::XMMatrixTranspose(screenToTranslatedWorld));
}