#pragma once

#include "../Macro/Macro.h"
#include "NDX11.h"


class NDX11RenderState
{
public:
	NDX11RenderState() {}
	virtual ~NDX11RenderState() {}
};

class NDX11DepthStencilState : public NDX11RenderState
{
	friend class NDX11;
public:
	NDX11DepthStencilState()
		: m_pState(NULL)
	{}

	virtual ~NDX11DepthStencilState()
	{
		N_RELEASE(m_pState);
	}

protected:
	ID3D11DepthStencilState* m_pState;
};

class NDX11RasterizerState : public NDX11RenderState
{
	friend class NDX11;
public:
	NDX11RasterizerState()
		: m_pState(NULL)
	{}

	virtual ~NDX11RasterizerState()
	{
		N_RELEASE(m_pState);
	}

protected:
	ID3D11RasterizerState* m_pState;
};

class NDX11BlendState : public NDX11RenderState
{
	friend class NDX11;
public:
	NDX11BlendState()
		: m_pState(NULL)
	{}

	virtual ~NDX11BlendState()
	{
		N_RELEASE(m_pState);
	}

protected:
	ID3D11BlendState* m_pState;
};

class NDX11SamplersState : public NDX11RenderState
{
	friend class NDX11;
public:
	NDX11SamplersState()
	{
		m_pState.clear();
	}

	virtual ~NDX11SamplersState()
	{
		for (auto it = m_pState.begin(); it != m_pState.end(); it++)
		{
			ID3D11SamplerState* clearState = *it;
			N_RELEASE(clearState);
		}
		m_pState.clear();
	}

protected:
	std::vector<ID3D11SamplerState*> m_pState;
};