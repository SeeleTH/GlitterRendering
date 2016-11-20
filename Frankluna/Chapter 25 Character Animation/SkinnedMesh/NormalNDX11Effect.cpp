#include "NormalNDX11Effect.h"

#include "../../../NBoxLib/Source/Graphics/NRenderCmd.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NShaderNResLoader.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"

#include "../../Common/GeometryGenerator.h"

UINT32 NormalNDX11Effect::g_u32FlakesNumber = 1000000;

NormalNDX11Effect::NormalNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap)
{
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

	NRes vsFile("FX\\NormalMap_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(layout);

	NRes psFile("FX\\NormalMap_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	NRes vsShadowFile("FX\\NormalMapShadow_vs.cso");
	m_pShadowVS = resCache->GetHandle(&vsShadowFile);
	((NVertexShaderNResExtraData*)m_pShadowVS->GetExtra().get())->Initialize(layout);

	NRes psShadowFile("FX\\NormalMapShadow_ps.cso");
	m_pShadowPS = resCache->GetHandle(&psShadowFile);

	NRes bricksDiffFile("Textures\\bricks.dds");
	NRes bricksNormFile("Textures\\bricks_nmap.dds");
	NRes stonesDiffFile("Textures\\stones.dds");
	NRes stonesNormFile("Textures\\stones_nmap.dds");
	NRes floorDiffFile("Textures\\floor.dds");
	NRes floorNormFile("Textures\\floor_nmap.dds");

	m_pTextures.push_back(cubeMap);
	m_pTextures.push_back(resCache->GetHandle(&bricksDiffFile));
	m_pTextures.push_back(resCache->GetHandle(&bricksNormFile));
	m_pTextures.push_back(resCache->GetHandle(&stonesDiffFile));
	m_pTextures.push_back(resCache->GetHandle(&stonesNormFile));
	m_pTextures.push_back(resCache->GetHandle(&floorDiffFile));
	m_pTextures.push_back(resCache->GetHandle(&floorNormFile));

	// Glitter
	NRes vsGlitterCountFile("FX\\GlitterCount_vs.cso");
	m_pGlitterCountVS = resCache->GetHandle(&vsGlitterCountFile);
	((NVertexShaderNResExtraData*)m_pGlitterCountVS->GetExtra().get())->Initialize();
	NRes psGlitterCountFile("FX\\GlitterCount_ps.cso");
	m_pGlitterCountPS = resCache->GetHandle(&psGlitterCountFile);
	NRes gsGlitterCountFile("FX\\GlitterCount_gs.cso");
	m_pGlitterCountGS = resCache->GetHandle(&gsGlitterCountFile);
	NRes csGlitterCountFile("FX\\GlitterCompute_cs.cso");
	m_pGlitterComputeCS = resCache->GetHandle(&csGlitterCountFile);


	// Generate vertex and index buffer
	BuildBuffers(dev);

	VSetWorld(DirectX::XMMatrixIdentity());
	VSetView(DirectX::XMMatrixIdentity());
	VSetProjection(DirectX::XMMatrixIdentity());

	m_sBufferPerApp.m_bAlphaClip = false;
	m_sBufferPerApp.m_bFogEnabled = false;
	m_sBufferPerApp.m_bReflectionEnabled = false;
	m_sBufferPerApp.m_bUseTexture = true;
	m_sBufferPerApp.m_iLightCount = 3;

	m_sBufferPerFrame.m_dirLight[0].Ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_sBufferPerFrame.m_dirLight[0].Diffuse = DirectX::XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f);
	m_sBufferPerFrame.m_dirLight[0].Specular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	m_sBufferPerFrame.m_dirLight[0].Direction = DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	m_sBufferPerFrame.m_dirLight[1].Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_sBufferPerFrame.m_dirLight[1].Diffuse = DirectX::XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	m_sBufferPerFrame.m_dirLight[1].Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_sBufferPerFrame.m_dirLight[1].Direction = DirectX::XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_sBufferPerFrame.m_dirLight[2].Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_sBufferPerFrame.m_dirLight[2].Diffuse = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_sBufferPerFrame.m_dirLight[2].Specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_sBufferPerFrame.m_dirLight[2].Direction = DirectX::XMFLOAT3(0.0f, 0.0, -1.0f);

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

	gridData.m_buffer.m_material.Ambient = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	gridData.m_buffer.m_material.Diffuse = DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	gridData.m_buffer.m_material.Specular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	gridData.m_buffer.m_material.Reflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	for (UINT32 i = 0; i < 10; i++)
	{
		cylinderData[i].m_buffer.m_material.Ambient = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		cylinderData[i].m_buffer.m_material.Diffuse = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		cylinderData[i].m_buffer.m_material.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
		cylinderData[i].m_buffer.m_material.Reflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		sphereData[i].m_buffer.m_material.Ambient = DirectX::XMFLOAT4(0.3f, 0.4f, 0.5f, 1.0f);
		sphereData[i].m_buffer.m_material.Diffuse = DirectX::XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
		sphereData[i].m_buffer.m_material.Specular = DirectX::XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
		sphereData[i].m_buffer.m_material.Reflect = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	}

	boxData.m_buffer.m_material.Ambient = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	boxData.m_buffer.m_material.Diffuse = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	boxData.m_buffer.m_material.Specular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	boxData.m_buffer.m_material.Reflect = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	gridData.m_uIndexCount = m_u32GridIndexCount;
	gridData.m_uIndexOffset = m_u32GridIndexOffset;
	gridData.m_uVertexOffset = m_u32GridVertexOffset;
	gridData.m_uCubeMapIndex = 0;
	gridData.m_uDiffuseMapIndex = 5;
	gridData.m_uNormalMapIndex = 6;

	boxData.m_uIndexCount = m_u32BoxIndexCount;
	boxData.m_uIndexOffset = m_u32BoxIndexOffset;
	boxData.m_uVertexOffset = m_u32BoxVertexOffset;
	boxData.m_uCubeMapIndex = 0;
	boxData.m_uDiffuseMapIndex = 1;
	boxData.m_uNormalMapIndex = 2;

	for (UINT32 i = 0; i < 10; i++)
	{
		cylinderData[i].m_uIndexCount = m_u32CylinderIndexCount;
		cylinderData[i].m_uIndexOffset = m_u32CylinderIndexOffset;
		cylinderData[i].m_uVertexOffset = m_u32CylinderVertexOffset;
		cylinderData[i].m_uCubeMapIndex = 0;
		cylinderData[i].m_uDiffuseMapIndex = 1;
		cylinderData[i].m_uNormalMapIndex = 2;

		sphereData[i].m_uIndexCount = m_u32SphereIndexCount;
		sphereData[i].m_uIndexOffset = m_u32SphereIndexOffset;
		sphereData[i].m_uVertexOffset = m_u32SphereVertexOffset;
		sphereData[i].m_uCubeMapIndex = 0;
		sphereData[i].m_uDiffuseMapIndex = 3;
		sphereData[i].m_uNormalMapIndex = 4;
	}

	m_sRenderItems.push_back(gridData);
	m_sRenderItems.push_back(boxData);
	for (UINT32 i = 0; i < 10; i++)
	{
		m_sRenderItems.push_back(cylinderData[i]);
		m_sRenderItems.push_back(sphereData[i]);
	}


	m_sSceneBound.m_vCenter = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_sSceneBound.m_fRadius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	// ================================
	// Shadow map
	// ================================
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

	BuildShadowTransform();
}

NormalNDX11Effect::~NormalNDX11Effect()
{
	N_DELETE(m_pShadowMap);
	N_DELETE(m_pFlakesCountMap);

	m_sRenderItems.clear();
	m_pTextures.clear();
}


void NormalNDX11Effect::VApplyToQueue(NRenderCmdList* list)
{
	UpdateBuffers();

	list->CmdSetViewports(1, &m_sShadowViewport);
	NTexture2DRes* renderTarget[1] = {0};
	list->CmdSetRendertargets(1, renderTarget, m_pShadowMap);
	list->CmdClear(N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER, nullptr, m_pShadowMap, 1.f);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_depthRS);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pShadowVS->GetExtra().get())->GetShader());
	list->CmdSetVertexBuffer(0, 1, &m_vertBuff);
	list->CmdSetIndexBuffer(&m_indBuff);
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pShadowPS->GetExtra().get())->GetShader());
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetConstantBufferData(&m_cShadowBufferPerFrame, &m_sShadowBufferPerFrame, sizeof(SHADOW_PERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cShadowBufferPerFrame);
	for (auto it = m_sRenderItems.begin(); it != m_sRenderItems.end(); it++)
	{
		list->CmdSetConstantBufferData(&m_cShadowBufferPerObject, &it->m_shadowBuffer, sizeof(SHADOW_PEROBJECT));
		list->CmdSetVSConstantBuffer(1, 1, &m_cShadowBufferPerObject);
		list->CmdSetPSTextureResource(0, (it->m_uDiffuseMapIndex >= 0) ? ((NDDSTextureNResExtraData*)m_pTextures[it->m_uDiffuseMapIndex]->GetExtra().get())->GetTexture() : NULL);
		list->CmdDrawIndexed(it->m_uIndexCount, it->m_uIndexOffset, it->m_uVertexOffset);
	}

	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { NGraphicsDevice::g_pMainGraphicsDevice->GetMainRenderTargets() };
	list->CmdSetRendertargets(1, defRenderTarget, NGraphicsDevice::g_pMainGraphicsDevice->GetMainDepthStencil());
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_shadowSampler, 1, 1);
	list->CmdSetVertexBuffer(0, 1, &m_vertBuff);
	list->CmdSetIndexBuffer(&m_indBuff);
	list->CmdSetConstantBufferData(&m_cBufferPerApp, &m_sBufferPerApp, sizeof(CPPERAPP));
	list->CmdSetPSConstantBuffer(2, 1, &m_cBufferPerApp);
	list->CmdSetConstantBufferData(&m_cBufferPerFrame, &m_sBufferPerFrame, sizeof(CPPERFRAME));
	list->CmdSetVSConstantBuffer(0, 1, &m_cBufferPerFrame);
	list->CmdSetPSConstantBuffer(0, 1, &m_cBufferPerFrame);
	list->CmdSetPSTextureResource(0, m_pShadowMap);

	for (auto it = m_sRenderItems.begin(); it != m_sRenderItems.end(); it++)
	{
		list->CmdSetConstantBufferData(&m_cBufferPerObject, &it->m_buffer, sizeof(CPPEROBJECT));
		list->CmdSetVSConstantBuffer(1, 1, &m_cBufferPerObject);
		list->CmdSetPSConstantBuffer(1, 1, &m_cBufferPerObject);
		list->CmdSetPSTextureResource(4, (it->m_uCubeMapIndex >= 0) ? ((NDDSTextureNResExtraData*)m_pTextures[it->m_uCubeMapIndex]->GetExtra().get())->GetTexture() : NULL);
		list->CmdSetPSTextureResource(1, (it->m_uDiffuseMapIndex >= 0) ? ((NDDSTextureNResExtraData*)m_pTextures[it->m_uDiffuseMapIndex]->GetExtra().get())->GetTexture() : NULL);
		list->CmdSetPSTextureResource(2, (it->m_uNormalMapIndex >= 0) ? ((NDDSTextureNResExtraData*)m_pTextures[it->m_uNormalMapIndex]->GetExtra().get())->GetTexture() : NULL);
		list->CmdDrawIndexed(it->m_uIndexCount, it->m_uIndexOffset, it->m_uVertexOffset);
	}

	// Glitter
	list->CmdSetDefViewports();
	list->CmdClear(N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER, nullptr, m_pFlakesCountMap, 1.f, 0U, 0.f, 0.f, 0.f);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);
	float glitterBlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	list->CmdSetBlendState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_particleBS, glitterBlendFactor, 0xffffffff);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pGlitterCountVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pGlitterCountPS->GetExtra().get())->GetShader());
	list->CmdSetGeometryShader(((NGeometryShaderNResExtraData*)m_pGlitterCountGS->GetExtra().get())->GetShader());
	list->CmdSetConstantBufferData(&m_cGlitterBufferPerFrame, &m_glitterPerFrame, sizeof(CP_GLITTER_PERFRAME));
	list->CmdSetConstantBufferData(&m_cGlitterBufferPerObject, &m_glitterPerObject, sizeof(CP_GLITTER_PEROBJ));
	list->CmdSetVSConstantBuffer(0, 1, &m_cGlitterBufferPerObject);
	list->CmdSetVSStructureResource(0, &m_pFlakesDataMap);
	list->CmdSetGSConstantBuffer(0, 1, &m_cGlitterBufferPerObject);
	list->CmdSetGSConstantBuffer(1, 1, &m_cGlitterBufferPerFrame);
	list->CmdSetGSStructureResource(0, &m_pFlakesDataMap);
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	list->CmdDraw(g_u32FlakesNumber, 0);
	//NTexture2DRes* glitterRT[1] = { m_pFlakesCountMap };
	//list->CmdSetRendertargets(1, glitterRT, NULL);
	//list->CmdDraw(g_u32FlakesNumber, 0);
	list->CmdSetVertexShader(NULL);
	list->CmdSetPixelShader(NULL);
	list->CmdSetGeometryShader(NULL);
	list->CmdSetBlendState(NULL, glitterBlendFactor, 0xffffffff);
	// ----------------

	list->CmdSetPSTextureResource(0, NULL);
	list->CmdSetPSTextureResource(1, NULL);
	list->CmdSetPSTextureResource(2, NULL);
	list->CmdSetDepthStencilState(NULL, 0);
	list->CmdSetRasterizerState(NULL);

}


void XM_CALLCONV NormalNDX11Effect::VSetWorld(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matWorld, value);
}

void XM_CALLCONV NormalNDX11Effect::VSetView(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matView, value);
}

void XM_CALLCONV NormalNDX11Effect::VSetProjection(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matProjection, value);
}

void NormalNDX11Effect::BuildBuffers(NGraphicsDevice* dev)
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

	dev->CreateVertexBuffer(m_vertBuff, N_B_USAGE_IMMUTABLE, sizeof(PosNormalTexTan)
		, 0, 0, &vertices[0], vertices.size(), sizeof(PosNormalTexTan), 0);

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	dev->CreateIndexBuffer(m_indBuff, N_B_USAGE_IMMUTABLE, sizeof(UINT)
		, 0, 0, &indices[0], totalIndexCount, N_R_FORMAT_R32_UINT, 0);


	dev->CreateConstantBuffer(m_cBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(CPPERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CPPEROBJECT), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cBufferPerApp, N_B_USAGE_DYNAMIC, sizeof(CPPERAPP), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cShadowBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(SHADOW_PERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cShadowBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(SHADOW_PEROBJECT), 1, D3D11_CPU_ACCESS_WRITE, 0);

	BuildGlitterBuffer(dev);
}

void NormalNDX11Effect::UpdateBuffers()
{
	for (auto it = m_sRenderItems.begin(); it != m_sRenderItems.end(); it++)
	{
		UpdateBuffer(*it);
	}

	DirectX::XMMATRIX matView = DirectX::XMLoadFloat4x4(&m_matView);
	DirectX::XMVECTOR viewDet = DirectX::XMMatrixDeterminant(matView);
	matView = DirectX::XMMatrixInverse(&viewDet, matView);
	DirectX::XMStoreFloat3(&m_sBufferPerFrame.m_f3EyePosW, matView.r[3]);
	DirectX::XMStoreFloat3(&m_sShadowBufferPerFrame.m_d3EyePosW, matView.r[3]);

	// glitter
	{
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX wvp = world * DirectX::XMLoadFloat4x4(&m_matView) * DirectX::XMLoadFloat4x4(&m_matProjection);
		DirectX::XMStoreFloat4x4(&m_glitterPerObject.m_matWorld, DirectX::XMMatrixTranspose(world));
		DirectX::XMStoreFloat4x4(&m_glitterPerObject.m_matWorldViewProj, DirectX::XMMatrixTranspose(wvp));

		DirectX::XMMATRIX A = world;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		A = XMMatrixInverse(&det, A);
		DirectX::XMStoreFloat4x4(&m_glitterPerObject.m_matInvTranspose, A);

		DirectX::XMStoreFloat4x4(&m_glitterPerFrame.m_matInvView, DirectX::XMMatrixTranspose(matView));
		m_glitterPerFrame.m_f4LightDir = DirectX::XMFLOAT4(m_sBufferPerFrame.m_dirLight[0].Direction.x
			, m_sBufferPerFrame.m_dirLight[0].Direction.y
			, m_sBufferPerFrame.m_dirLight[0].Direction.z
			, 0.0f);
	}
}

void NormalNDX11Effect::UpdateBuffer(RENDERITEM& buffer)
{
	DirectX::XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	DirectX::XMMATRIX wvp;
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorld, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&buffer.m_matWorld)));
	wvp = DirectX::XMLoadFloat4x4(&buffer.m_matWorld)
		* DirectX::XMLoadFloat4x4(&m_matView)* DirectX::XMLoadFloat4x4(&m_matProjection);
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorldViewProj, DirectX::XMMatrixTranspose(wvp));
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matWorldViewProjTex
		, DirectX::XMMatrixTranspose(wvp * toTexSpace));
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matShadowTransform, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&buffer.m_matWorld) * DirectX::XMLoadFloat4x4(&m_matShadowTransform)));

	DirectX::XMMATRIX A = DirectX::XMLoadFloat4x4(&buffer.m_matWorld);
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
	A = XMMatrixInverse(&det, A);
	DirectX::XMStoreFloat4x4(&buffer.m_buffer.m_matInvTranspose, A);

	buffer.m_shadowBuffer.m_matWorld = buffer.m_buffer.m_matWorld;
	buffer.m_shadowBuffer.m_matInvTranspose = buffer.m_buffer.m_matInvTranspose;
	buffer.m_shadowBuffer.m_matTexTransform = buffer.m_buffer.m_matTexTransform;
	DirectX::XMMATRIX shadowViewProj = DirectX::XMLoadFloat4x4(&m_matLightView) * DirectX::XMLoadFloat4x4(&m_matLightProj);
	DirectX::XMMATRIX shadowWVP = DirectX::XMLoadFloat4x4(&buffer.m_matWorld) * shadowViewProj;
	DirectX::XMStoreFloat4x4(&buffer.m_shadowBuffer.m_matViewProj, DirectX::XMMatrixTranspose(shadowViewProj));
	DirectX::XMStoreFloat4x4(&buffer.m_shadowBuffer.m_matWorldViewProj, DirectX::XMMatrixTranspose(shadowWVP));
}

void NormalNDX11Effect::BuildShadowTransform()
{
	DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&m_sBufferPerFrame.m_dirLight[0].Direction);
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

	DirectX::XMStoreFloat4x4(&m_matLightView, view);
	DirectX::XMStoreFloat4x4(&m_matLightProj, proj);
	DirectX::XMStoreFloat4x4(&m_matShadowTransform, shadowTransform);
}

void NormalNDX11Effect::BuildGlitterBuffer(NGraphicsDevice* dev)
{
	srand(GetTickCount());
	GLITTER_FLAKE* flakesData = new GLITTER_FLAKE[g_u32FlakesNumber];
	for (UINT32 i = 0; i < g_u32FlakesNumber; i++)
	{
		float u = (float)(rand() % 1000) / 1000.f;
		float v = (float)(rand() % 1000) / 1000.f;
		//float phi = 2.0f * XM_PI * v;
		//float cosTheta = sqrt(1.0f - u);
		//float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
		//flakesData[i].m_f4Dir = { sin(phi) * sinTheta, cosTheta, cos(phi) * sinTheta, 0.f };
		v = v*0.25f;
		float phi = 2.f * XM_PI * u;
		float r = sqrt(v);
		float theta = acosf(r);
		flakesData[i].m_f4Dir = { r * cos(phi), sin(theta), r * sin(phi), 0.f };
		flakesData[i].m_f4Pos = { ((float)(rand() % 1000) / 1000.f) * 3.f - 1.5f
			, 2.f
			, ((float)(rand() % 1000) / 1000.f) * 3.f - 1.5f, 1.f };
	}

	dev->CreateStructureBuffer(m_pFlakesDataMap, N_B_USAGE_DEFAULT
		, sizeof(GLITTER_FLAKE), D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
		0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, flakesData, g_u32FlakesNumber);
	
	N_DELETE_ARRAY(flakesData);

	N_G_SHADER_RESOURCE_VIEW_DESC flakesDataDesc;
	ZeroMemory(&flakesDataDesc, sizeof(flakesDataDesc));
	flakesDataDesc.Format = DXGI_FORMAT_UNKNOWN;
	flakesDataDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	flakesDataDesc.Buffer.FirstElement = 0;
	flakesDataDesc.Buffer.NumElements = g_u32FlakesNumber;
	dev->CreateSRV(m_pFlakesDataMap, flakesDataDesc);

	dev->CreateConstantBuffer(m_cGlitterBufferPerFrame, N_B_USAGE_DYNAMIC, sizeof(CP_GLITTER_PERFRAME), 1, D3D11_CPU_ACCESS_WRITE, 0);
	dev->CreateConstantBuffer(m_cGlitterBufferPerObject, N_B_USAGE_DYNAMIC, sizeof(CP_GLITTER_PEROBJ), 1, D3D11_CPU_ACCESS_WRITE, 0);


	m_pFlakesCountMap = new NTexture2DRes();

	N_G_TEXTURE2D_DESC texDesc;
	texDesc.Width = dev->GetClientWidth();
	texDesc.Height = dev->GetClientHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	dev->CreateTextureFromDesc(*m_pFlakesCountMap, texDesc, NULL, 0);

	dev->CreateSRV(*m_pFlakesCountMap);
	dev->CreateRTV(*m_pFlakesCountMap);

	m_pFlakesCountMap->ReleaseTexture();
}