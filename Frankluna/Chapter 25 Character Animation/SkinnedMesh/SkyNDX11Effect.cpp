#include "SkyNDX11Effect.h"

#include "../../../NBoxLib/Source/Graphics/NRenderCmd.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NShaderNResLoader.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"

#include "../../Common/GeometryGenerator.h"

SkyNDX11Effect::SkyNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap)
	: m_pCubeMap(cubeMap)
{
	N_G_SHADER_LAYOUT layout;
	layout.numElements = 1;
	layout.layouts[0].SemanticName = "POSITION";
	layout.layouts[0].SemanticIndex = 0;
	layout.layouts[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout.layouts[0].InputSlot = 0;
	layout.layouts[0].AlignedByteOffset = 0;
	layout.layouts[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout.layouts[0].InstanceDataStepRate = 0;

	NRes vsFile("FX\\Sky_vs.cso");
	m_pVS = resCache->GetHandle(&vsFile);
	((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->Initialize(layout);

	NRes psFile("FX\\Sky_ps.cso");
	m_pPS = resCache->GetHandle(&psFile);

	// Generate vertex and index buffer

	GeometryGenerator::MeshData sphere;
	GeometryGenerator geoGen;
	geoGen.CreateSphere(5000.f, 30, 30, sphere);

	std::vector<DirectX::XMFLOAT3> vertices(sphere.Vertices.size());

	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i].x = sphere.Vertices[i].Position.x;
		vertices[i].y = sphere.Vertices[i].Position.y;
		vertices[i].z = sphere.Vertices[i].Position.z;
	}

	dev->CreateVertexBuffer(m_vertBuff, N_B_USAGE_IMMUTABLE, sizeof(DirectX::XMFLOAT3)
		, 0, 0, &vertices[0], vertices.size(), sizeof(DirectX::XMFLOAT3), 0);

	m_uIndexCount = sphere.Indices.size();
	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

	dev->CreateIndexBuffer(m_indBuff, N_B_USAGE_IMMUTABLE, sizeof(USHORT)
		, 0, 0, &indices16[0], m_uIndexCount, N_R_FORMAT_R16_UINT, 0);

	dev->CreateConstantBuffer(m_cBufferWVP, N_B_USAGE_DYNAMIC, sizeof(DirectX::XMFLOAT4X4), 1, D3D11_CPU_ACCESS_WRITE, 0);

	VSetWorld(DirectX::XMMatrixIdentity());
	VSetView(DirectX::XMMatrixIdentity());
	VSetProjection(DirectX::XMMatrixIdentity());
}

SkyNDX11Effect::~SkyNDX11Effect()
{
}


void SkyNDX11Effect::VApplyToQueue(NRenderCmdList* list)
{
	list->CmdSetDefViewports();
	NTexture2DRes* defRenderTarget[1] = { NGraphicsDevice::g_pMainGraphicsDevice->GetMainRenderTargets() };
	list->CmdSetRendertargets(1, defRenderTarget, NGraphicsDevice::g_pMainGraphicsDevice->GetMainDepthStencil());

	DirectX::XMMATRIX wvp = DirectX::XMLoadFloat4x4(&m_matWorld) * DirectX::XMLoadFloat4x4(&m_matView)* DirectX::XMLoadFloat4x4(&m_matProjection);
	DirectX::XMStoreFloat4x4(&m_matWVP, DirectX::XMMatrixTranspose(wvp));
	list->CmdSetConstantBufferData(&m_cBufferWVP, &m_matWVP, sizeof(DirectX::XMFLOAT4X4)); // <- a bit scary here. (m_matWVP will changed need to do something)

	list->CmdSetDepthStencilState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_lessEqualsDSS, 0);
	list->CmdSetRasterizerState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_noCullRS);
	list->CmdSetVSConstantBuffer(0, 1, &m_cBufferWVP);
	list->CmdSetPSTextureResource(0, ((NDDSTextureNResExtraData*)m_pCubeMap->GetExtra().get())->GetTexture());
	list->CmdSetVertexBuffer(0, 1, &m_vertBuff);
	list->CmdSetIndexBuffer(&m_indBuff);
	list->CmdSetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->CmdSetVertexShader(((NVertexShaderNResExtraData*)m_pVS->GetExtra().get())->GetShader());
	list->CmdSetPixelShader(((NPixelShaderNResExtraData*)m_pPS->GetExtra().get())->GetShader());
	list->CmdSetPSSamplersState(&NDX11::g_pMainGraphicsDevice->GetRenderStateManager()->m_linearSampler, 0, 1);
	list->CmdDrawIndexed(m_uIndexCount, 0, 0);
}


void XM_CALLCONV SkyNDX11Effect::VSetWorld(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matWorld, value);
}

void XM_CALLCONV SkyNDX11Effect::VSetView(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matView, value);
}

void XM_CALLCONV SkyNDX11Effect::VSetProjection(DirectX::FXMMATRIX value)
{
	DirectX::XMStoreFloat4x4(&m_matProjection, value);
}
