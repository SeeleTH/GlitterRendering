#pragma once
#include "../Macro/Macro.h"
#include "NDX11.h"

class NDX11Buffer
{
public:
	NDX11Buffer(ID3D11Buffer* buffer = nullptr);
    virtual ~NDX11Buffer();
    virtual void VOnPreRender() {}
    virtual void VOnPostRender() {}

    ID3D11Buffer* GetBuffer() { return m_pBuffer; }

protected:
    ID3D11Buffer* m_pBuffer;

	friend class NDX11;
};

class NDX11VertexBuffer : public NDX11Buffer
{
public:
	NDX11VertexBuffer(ID3D11Buffer* buffer = nullptr, UINT32 stride = 0, UINT32 offset = 0);
    virtual ~NDX11VertexBuffer();
    virtual void VOnPreRender();
    virtual void VOnPostRender();

protected:
    UINT32 m_iStride;
    UINT32 m_iOffset;

	friend class NDX11;
};

class NDX11IndexBuffer : public NDX11Buffer
{
public:
	NDX11IndexBuffer(ID3D11Buffer* buffer = nullptr, N_RESOURCE_FORMAT m_eFormat = N_R_FORMAT_R16_UINT, UINT32 offset = 0);
    virtual ~NDX11IndexBuffer();
    virtual void VOnPreRender();
    virtual void VOnPostRender();

protected:
    N_RESOURCE_FORMAT m_eFormat;
    UINT32 m_iOffset;

	friend class NDX11;
};

class NDX11ConstantBuffer : public NDX11Buffer
{
public:
	NDX11ConstantBuffer(ID3D11Buffer* buffer = nullptr);
    virtual ~NDX11ConstantBuffer();
    virtual void VOnPreRender();
    virtual void VOnPostRender();

protected:
	friend class NDX11;
};

class NDX11StructureBuffer : public NDX11Buffer
{
public:
	NDX11StructureBuffer(ID3D11Buffer* buffer = nullptr);
	virtual ~NDX11StructureBuffer();
	virtual void VOnPreRender();
	virtual void VOnPostRender();

	inline ID3D11ShaderResourceView* GetSRV() { return m_pSRV; }
	inline ID3D11UnorderedAccessView* GetUAV() { return m_pUAV; }

protected:
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11UnorderedAccessView* m_pUAV;

	friend class NDX11;
};