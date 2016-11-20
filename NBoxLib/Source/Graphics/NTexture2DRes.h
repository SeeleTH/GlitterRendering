#pragma once

#include "NTextureBase.h"
#include "NDX11.h"

//class INDX11Resource
//{
//public:
//	virtual ID3D11Resource* VGetResource() = 0;
//	virtual void VSetResource(ID3D11Resource* res) = 0;
//	virtual void VReleaseResource() = 0;
//};

class NDX11Texture2DRes : public NTextureBase
{
	friend class NDX11;
public:
    NDX11Texture2DRes();
    ~NDX11Texture2DRes();

    inline const ID3D11Texture2D* GetTexture() const { return m_pTexture; }
    inline const ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }
    inline const ID3D11DepthStencilView* GetDSV() const { return m_pDSV; }
	inline const ID3D11RenderTargetView* GetRTV() const { return m_pRTV; }
	inline const ID3D11UnorderedAccessView* GetUAV() const { return m_pUAV; }

    inline ID3D11Texture2D* GetTexture() { return m_pTexture; }
    inline ID3D11ShaderResourceView* GetSRV() { return m_pSRV; }
    inline ID3D11DepthStencilView* GetDSV() { return m_pDSV; }
	inline ID3D11RenderTargetView* GetRTV() { return m_pRTV; }
	inline ID3D11UnorderedAccessView* GetUAV() { return m_pUAV; }

	inline void SetTexture(ID3D11Texture2D* texture) { m_pTexture = texture; }
	inline void SetDevice(NDX11* dev) { m_pDevice = dev; }
	inline void SetDSV(ID3D11DepthStencilView* dsv) { m_pDSV = dsv; }
	inline void SetRTV(ID3D11RenderTargetView* rtv) { m_pRTV = rtv; }
	inline void SetUAV(ID3D11UnorderedAccessView* uav) { m_pUAV = uav; }

	void ReleaseTexture();
	void GetDesc(N_G_TEXTURE2D_DESC& desc);

protected:

    NDX11* m_pDevice;
    ID3D11Texture2D* m_pTexture;
    ID3D11ShaderResourceView* m_pSRV;
    ID3D11DepthStencilView* m_pDSV;
    ID3D11RenderTargetView* m_pRTV;
	ID3D11UnorderedAccessView* m_pUAV;
};

typedef NDX11Texture2DRes NTexture2DRes;