#include "NDX11Buffer.h"
#include "NDX11.h"


NDX11Buffer::NDX11Buffer(ID3D11Buffer* buffer)
:m_pBuffer(buffer)
{

}

NDX11Buffer::~NDX11Buffer()
{
    N_RELEASE(m_pBuffer);
}

NDX11VertexBuffer::NDX11VertexBuffer(ID3D11Buffer* buffer, UINT32 stride, UINT32 offset)
	: NDX11Buffer(buffer)
	, m_iStride(stride)
	, m_iOffset(offset)
{

}

NDX11VertexBuffer::~NDX11VertexBuffer()
{

}

void NDX11VertexBuffer::VOnPreRender()
{
}

void NDX11VertexBuffer::VOnPostRender()
{
}

NDX11IndexBuffer::NDX11IndexBuffer(ID3D11Buffer* buffer, N_RESOURCE_FORMAT m_eFormat, UINT32 offset)
	: NDX11Buffer(buffer)
	, m_eFormat(m_eFormat)
	, m_iOffset(offset)
{

}

NDX11IndexBuffer::~NDX11IndexBuffer()
{

}

void NDX11IndexBuffer::VOnPreRender()
{
}

void NDX11IndexBuffer::VOnPostRender()
{
}

NDX11ConstantBuffer::NDX11ConstantBuffer(ID3D11Buffer* buffer)
	: NDX11Buffer(buffer)
{

}

NDX11ConstantBuffer::~NDX11ConstantBuffer()
{

}

void NDX11ConstantBuffer::VOnPreRender()
{

}

void NDX11ConstantBuffer::VOnPostRender()
{

}

NDX11StructureBuffer::NDX11StructureBuffer(ID3D11Buffer* buffer)
	: NDX11Buffer(buffer)
	, m_pSRV(nullptr)
	, m_pUAV(nullptr)
{

}

NDX11StructureBuffer::~NDX11StructureBuffer()
{
	N_RELEASE(m_pSRV);
	N_RELEASE(m_pUAV);
}

void NDX11StructureBuffer::VOnPreRender()
{

}

void NDX11StructureBuffer::VOnPostRender()
{

}