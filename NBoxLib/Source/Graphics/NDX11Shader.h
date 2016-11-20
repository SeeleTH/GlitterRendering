#pragma once
#include "../Macro/Macro.h"
#include "../Template/NIDGenerator.h"
#include "NDX11.h"

class NDX11VertexShader
{
public:
	NDX11VertexShader(ID3D11VertexShader* vertexShader = nullptr, ID3D11InputLayout* inputLayout = nullptr)
		: m_pVertexShader(vertexShader)
		, m_pInputLayout(inputLayout)
	{}

protected:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;

	friend class NDX11;
};

class NDX11PixelShader
{
public:
	NDX11PixelShader(ID3D11PixelShader* pixelShader = nullptr)
		: m_pPixelShader(pixelShader)
	{}

protected:
	ID3D11PixelShader* m_pPixelShader;

	friend class NDX11;
};

class NDX11GeometryShader
{
public:
	NDX11GeometryShader(ID3D11GeometryShader* geometryShader = nullptr)
		: m_pGeometryShader(geometryShader)
	{}

protected:
	ID3D11GeometryShader* m_pGeometryShader;
	friend class NDX11;
};

class NDX11ComputeShader
{
public:
	NDX11ComputeShader(ID3D11ComputeShader* computeShader = nullptr)
		: m_pComputeShader(computeShader)
	{}

protected:
	ID3D11ComputeShader* m_pComputeShader;
	friend class NDX11;
};

class INSEShader
{
public:
	CLASS_ID_GEN_FUNC(INSEShader, INSEShader)
	virtual ~INSEShader(){}
	virtual BOOL VInitShader() { return true; }
	virtual BOOL VPreSetShader() { return true; }
	virtual BOOL VPostSetShader() { return true; }
};