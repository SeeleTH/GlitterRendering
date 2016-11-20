#pragma once
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include <memory>
#include <DirectXMath.h>

class NResCache;
class NResHandle;

class SkyNDX11Effect : public INDX11Effect, INDX11EffectMatrices
{
public:
	SkyNDX11Effect(NGraphicsDevice* dev, NResCache* resCache, std::shared_ptr<NResHandle> cubeMap);
	~SkyNDX11Effect();

	void VApplyToQueue(NRenderCmdList* list) override;

	void XM_CALLCONV VSetWorld(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetView(DirectX::FXMMATRIX value) override;
	void XM_CALLCONV VSetProjection(DirectX::FXMMATRIX value) override;

protected:
	DirectX::XMFLOAT4X4 m_matWorld;
	DirectX::XMFLOAT4X4 m_matView;
	DirectX::XMFLOAT4X4 m_matProjection;
	DirectX::XMFLOAT4X4 m_matWVP;

	NVertexBuffer m_vertBuff;
	NIndexBuffer m_indBuff;
	UINT32 m_uIndexCount;
	NConstantBuffer m_cBufferWVP;
	NConstantBuffer m_tBufferCubemap;

	std::shared_ptr<NResHandle> m_pCubeMap;
	std::shared_ptr<NResHandle> m_pVS;
	std::shared_ptr<NResHandle> m_pPS;
};