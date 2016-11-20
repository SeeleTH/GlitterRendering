#include "NTexture2DRes.h"


NDX11Texture2DRes::NDX11Texture2DRes()
    : m_pTexture(NULL)
    , m_pSRV(NULL)
    , m_pDSV(NULL)
    , m_pRTV(NULL)
	, m_pUAV(NULL)
    , m_pDevice(NULL)
{

}

NDX11Texture2DRes::~NDX11Texture2DRes()
{
    N_RELEASE(m_pRTV);
    N_RELEASE(m_pSRV);
    N_RELEASE(m_pDSV);
	N_RELEASE(m_pUAV);
    N_RELEASE(m_pTexture);
}

void NDX11Texture2DRes::ReleaseTexture()
{
    N_RELEASE(m_pTexture);
}


void NDX11Texture2DRes::GetDesc(N_G_TEXTURE2D_DESC& desc)
{
	N_ASSERT(m_pTexture || m_pSRV);
	if (!m_pTexture)
	{
		ID3D11Texture2D* texture = nullptr;
		ID3D11Resource* res = nullptr;
		m_pSRV->GetResource(&res);
		res->QueryInterface(&texture);
		texture->GetDesc(&desc);
		N_RELEASE(texture);
		N_RELEASE(res);
	}
	else
	{
		m_pTexture->GetDesc(&desc);
	}

}