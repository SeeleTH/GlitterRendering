#pragma once

#include "../Macro/Macro.h"
#include "../Template/NNonCopyable.h"
#include "NGraphics.h"

//class NRenderCmd
//{
//public:
//	enum COMMAND_TYPE
//	{
//		CLEAR_BUFFERS,
//		CLEAR_RENDERTARGET,
//		CLEAR_DEPTHSTENCIL,
//		PRESENT,
//		SET_VIEWPORTS,
//		SET_RENDERTARGETS,
//		SET_RASTERIZERSTATE,
//		SET_DEPTHSTENCILSTATE,
//		SET_BLENDSTATE,
//		COMMANDS_NO
//	};
//
//	struct COMMAND
//	{
//		UINT32 uId;
//		UINT32 uSize;
//	};
//
//	struct CMD_CLEAR_BUFFERS : public COMMAND
//	{
//		UINT32 u32ClearMask;
//	};
//
//	struct CMD_CLEAR_RENDERTARGET : public COMMAND
//	{
//		NTexture2DRes* pRenderTarget;
//		float fColor[4];
//	};
//
//	struct CMD_CLEAR_DEPTHSTENCIL : public COMMAND
//	{
//		NTexture2DRes* pDepthStencil;
//		UINT32 u32ClearFlags;
//		float fDepth;
//		UINT8 u8Stencil;
//		UINT8 u8Pading[3];
//	};
//
//	struct CMD_PRESENT : public COMMAND
//	{
//	};
//
//	struct CMD_SET_VIEWPORTS : public COMMAND
//	{
//		UINT32 u32ViewPortNum;
//		N_G_VIEWPORT pViewports[N_G_MAX_VIEWPORTS];
//	};
//
//	struct CMD_SET_RENDERTARGETS : public COMMAND
//	{
//		UINT32 u32ViewNum;
//		NTexture2DRes* pRenderTargets[N_G_MAX_RENDERTARGETVIEWS];
//		NTexture2DRes* pDepthStencil;
//	};
//
//	struct CMD_SET_RASTERIZERSTATE : public COMMAND
//	{
//		NRasterizerState* pState;
//	};
//
//	struct CMD_SET_DEPTHSTENCILSTATE : public COMMAND
//	{
//		NDepthStencilState* pState;
//		UINT32 u32StencilRef;
//	};
//
//	struct CMD_SET_BLENDSTATE : public COMMAND
//	{
//		NBlendState* pState;
//		float fBlendFactors[4];
//		UINT32 u32SampleMask;
//	};
//
//protected:
//	typedef void(*PFnSubmit)(NGraphicsDevice&, COMMAND&);
//	static PFnSubmit g_CommandTable[];
//
//	typedef void(*PFnDelete)(COMMAND&);
//	static PFnDelete g_CommandDeleteTable[];
//
//public:
//
//	static inline void SubmitCommand(NGraphicsDevice& dev, COMMAND& cmd)
//	{
//		g_CommandTable[cmd.uId](dev, cmd);
//	}
//
//	static inline void DeleteCommand(COMMAND& cmd)
//	{
//		g_CommandDeleteTable[cmd.uId](cmd);
//	}
//
//protected:
//	//============================
//	// Commands
//	//============================
//
//	static void Submit_ClearBuffers(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_ClearRenderTarget(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_ClearDepthStencil(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_Present(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_SetViewPorts(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_SetRenderTargets(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_SetRasterizerState(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_SetDepthStencilState(NGraphicsDevice& dev, COMMAND& cmd);
//	static void Submit_SetBlendState(NGraphicsDevice& dev, COMMAND& cmd);
//
//#define DELETE_CMD_FUNC_MACRO(__CMDNAME__ ,__CMDTYPE__ ) static inline void Delete_##__CMDNAME__(COMMAND& cmd) { __CMDTYPE__* pCmd = (__CMDTYPE__ *)&cmd; N_DELETE(pCmd); }
//	DELETE_CMD_FUNC_MACRO(ClearBuffers,CMD_CLEAR_BUFFERS);
//	DELETE_CMD_FUNC_MACRO(ClearRenderTarget,CMD_CLEAR_RENDERTARGET);
//	DELETE_CMD_FUNC_MACRO(ClearDepthStencil,CMD_CLEAR_DEPTHSTENCIL);
//	DELETE_CMD_FUNC_MACRO(Present,CMD_PRESENT);
//	DELETE_CMD_FUNC_MACRO(SetViewPorts,CMD_SET_VIEWPORTS);
//	DELETE_CMD_FUNC_MACRO(SetRenderTargets,CMD_SET_RENDERTARGETS);
//	DELETE_CMD_FUNC_MACRO(SetRasterizerState,CMD_SET_RASTERIZERSTATE);
//	DELETE_CMD_FUNC_MACRO(SetDepthStencilState,CMD_SET_DEPTHSTENCILSTATE);
//	DELETE_CMD_FUNC_MACRO(SetBlendState,CMD_SET_BLENDSTATE);
//};
//
//class NRenderCmdList
//{
//
//};

class NRenderCmdListBase;

struct NRenderCmdBase
{
	NRenderCmdBase* pNext;
	void(*fExecuteAndDestructPtr)(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList, NRenderCmdBase *Cmd);
	
	inline NRenderCmdBase(void(*InExecuteAndDestructPtr)(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList, NRenderCmdBase *Cmd))
		: pNext(nullptr)
		, fExecuteAndDestructPtr(InExecuteAndDestructPtr)
	{
	}

	// Temporary -> Need memory manager at cmdlist class
	virtual ~NRenderCmdBase()
	{
	}

	inline void CallExecuteAndDestruct(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		fExecuteAndDestructPtr(Dev, CmdList, this);
	}
};

class NRenderCmdListIterator;

class NRenderCmdListBase : public NNonCopyable
{
public:
	NRenderCmdListBase()
	{
		Reset();
	}

	~NRenderCmdListBase()
	{
		Flush();
	}

	inline void Flush();
	inline void ExecuteList(NGraphicsDevice& dev);

	// Temporary -> Need mem manager
	template<typename T>
	inline void* Alloc()
	{
		return new T();
	}

	template<typename TCmd>
	inline void* AllocCommand()
	{
		TCmd* result = (TCmd*)Alloc<TCmd>();
		m_uNumCommands++;

		if (m_pCommandLink != nullptr)
			m_pCommandLink->pNext = result;
		m_pCommandLink = result;

		if (m_pRoot == nullptr)
			m_pRoot = result;

		return result;
	}

	inline BOOL HasCommands() const
	{
		return m_uNumCommands > 0;
	}

	inline void MoveCommandsTo(NRenderCmdListBase& other)
	{
		if (!other.HasCommands())
		{
			other.m_pRoot = m_pRoot;
			other.m_pCommandLink = m_pCommandLink;
		}
		else
		{
			other.m_pCommandLink->pNext = m_pRoot;
		}

		other.m_uNumCommands += m_uNumCommands;

		m_pRoot = m_pCommandLink = nullptr;
		m_uNumCommands = 0;
	}

private:
	void Reset();

	NRenderCmdBase* m_pRoot;
	NRenderCmdBase* m_pCommandLink;
	UINT32 m_uNumCommands;

	friend class NRenderCmdListIterator;
};

class NRenderCmdListIterator
{
public:
	NRenderCmdListIterator(NRenderCmdListBase& CmdList)
	{
		m_pCmdPtr = CmdList.m_pRoot;
		m_uNumCommands = 0;
		m_uCmdListNumCommands = CmdList.m_uNumCommands;
	}

	~NRenderCmdListIterator()
	{
		// @TODO: Check missing cmd
	}

	inline BOOL HasCommandsLeft() const
	{
		return !!m_pCmdPtr;
	}

	inline NRenderCmdBase* NextCommand()
	{
		if (!HasCommandsLeft())
			return nullptr;

		NRenderCmdBase* Cmd = m_pCmdPtr;
		m_pCmdPtr = Cmd->pNext;
		m_uNumCommands++;
		return Cmd;
	}

private:
	NRenderCmdBase* m_pCmdPtr;
	UINT32 m_uNumCommands;
	UINT32 m_uCmdListNumCommands;
};

inline void NRenderCmdListBase::Flush()
{
	if (!HasCommands())
		return;

	NRenderCmdListIterator it(*this);
	while (it.HasCommandsLeft())
	{
		NRenderCmdBase* Cmd = it.NextCommand();
		if (Cmd)
			delete Cmd;
	}

	m_uNumCommands = 0;
}

inline void NRenderCmdListBase::ExecuteList(NGraphicsDevice& dev)
{
	if (!HasCommands())
		return;

	NRenderCmdListIterator it(*this);
	while (it.HasCommandsLeft())
	{
		NRenderCmdBase* Cmd = it.NextCommand();
		Cmd->CallExecuteAndDestruct(dev, *this);
		//delete Cmd;
	}
}

//////////////////////////////////////////////////////
// Real Implement
//////////////////////////////////////////////////////

template<typename TCmd>
struct NRenderCmd : public NRenderCmdBase
{
	inline NRenderCmd()
		: NRenderCmdBase(&ExecuteAndDestruct)
	{

	}

	static inline void ExecuteAndDestruct(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList, NRenderCmdBase *Cmd)
	{
		TCmd *ThisCmd = (TCmd*)Cmd;
		ThisCmd->Execute(Dev, CmdList);
		ThisCmd->~TCmd();
	}
};

struct NRenderCmdClear : public NRenderCmd < NRenderCmdClear >
{
	UINT32 m_uMask;
	NTexture2DRes* m_pRenderTarget;
	NTexture2DRes* m_pDepthStencilTarget;
	float m_fDepth;
	UINT32 m_uStencil;
	float m_fColorR;
	float m_fColorG;
	float m_fColorB;
	float m_fColorA;


	inline NRenderCmdClear(UINT32 mask = N_B_ALL
		, NTexture2DRes* renderTarget = nullptr
		, NTexture2DRes* depthStencilTarget = nullptr
		, float depth = -1.f, UINT32 stencil = 0
		, float colorR = -1.f, float colorG = -1.f, float colorB = -1.f, float colorA = -1.f)
		: m_uMask(mask)
		, m_pRenderTarget(renderTarget)
		, m_pDepthStencilTarget(depthStencilTarget)
		, m_fDepth(depth)
		, m_uStencil(stencil)
		, m_fColorR(colorR)
		, m_fColorG(colorG)
		, m_fColorB(colorB)
		, m_fColorA(colorA)
	{

	}

	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.Clear(m_uMask, (m_pRenderTarget) ? m_pRenderTarget->GetRTV() : nullptr
			, (m_pDepthStencilTarget) ? m_pDepthStencilTarget->GetDSV() : nullptr
			, m_fDepth, m_uStencil, m_fColorR, m_fColorG, m_fColorB, m_fColorA);
	}
};

struct NRenderCmdPresent : public NRenderCmd < NRenderCmdPresent >
{
	inline NRenderCmdPresent()
	{

	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.Present();
	}
};

// =============================================
// WARNNING WARNNING WARNNING WARNNING WARNNING 
//			Construction Area - Begin
// WARNNING WARNNING WARNNING WARNNING WARNNING 
// =============================================

struct NRenderCmdSetViewports : public NRenderCmd < NRenderCmdSetViewports >
{
	UINT32 m_uViewportsNum;
	N_G_VIEWPORT m_pViewports[N_G_MAX_VIEWPORTS];

	inline NRenderCmdSetViewports() {}
	inline NRenderCmdSetViewports(UINT32 viewportsNum, N_G_VIEWPORT viewports[N_G_MAX_VIEWPORTS])
		: m_uViewportsNum(viewportsNum)
	{
		for (UINT32 i = 0; i < m_uViewportsNum; i++)
			m_pViewports[i] = viewports[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetViewports(m_pViewports, m_uViewportsNum);
	}
};

struct NRenderCmdSetDefViewports : public NRenderCmd < NRenderCmdSetDefViewports >
{
	inline NRenderCmdSetDefViewports() {}

	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetViewports(Dev.GetScreenViewport(), 1);
	}
};

struct NRenderCmdSetRendertargets : public NRenderCmd < NRenderCmdSetRendertargets >
{
	UINT32 m_uViewsNum;
	NTexture2DRes* m_pRenderTargets[N_G_MAX_RENDERTARGETVIEWS];
	NTexture2DRes* m_pDepthStencil;

	inline NRenderCmdSetRendertargets() {}
	inline NRenderCmdSetRendertargets(UINT32 viewsnum, NTexture2DRes* rendertargets[N_G_MAX_RENDERTARGETVIEWS], NTexture2DRes* depthstencil)
		: m_uViewsNum(viewsnum)
		, m_pDepthStencil(depthstencil)
	{
		for (UINT32 i = 0; i < m_uViewsNum; i++)
			m_pRenderTargets[i] = rendertargets[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetRenderTargets(m_pRenderTargets, m_uViewsNum, m_pDepthStencil);
	}
};

struct NRenderCmdSetRasterizerState : public NRenderCmd < NRenderCmdSetRasterizerState >
{
	NRasterizerState* m_pState;

	inline NRenderCmdSetRasterizerState() {}
	inline NRenderCmdSetRasterizerState(NRasterizerState* state)
		: m_pState(state)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetRasterizerState(m_pState);
	}
};

struct NRenderCmdSetDepthStencilState : public NRenderCmd < NRenderCmdSetDepthStencilState >
{
	NDepthStencilState* m_pState;
	UINT32 m_uStencilRef;

	inline NRenderCmdSetDepthStencilState() {}
	inline NRenderCmdSetDepthStencilState(NDepthStencilState* state, UINT32 stencilref)
		: m_pState(state)
		, m_uStencilRef(stencilref)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetDepthStencilState(m_pState, m_uStencilRef);
	}
};

struct NRenderCmdSetBlendState : public NRenderCmd < NRenderCmdSetBlendState >
{

	NBlendState* m_pState;
	float m_fBlendFactors[4];
	UINT32 m_uSampleMask;

	inline NRenderCmdSetBlendState() {}
	inline NRenderCmdSetBlendState(NBlendState* state, float blendFactors[4], UINT32 sampleMask)
		: m_pState(state)
		, m_uSampleMask(sampleMask)
	{
		for (UINT32 i = 0; i < 4; i++)
			m_fBlendFactors[i] = blendFactors[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetBlendStateState(m_pState,m_fBlendFactors, m_uSampleMask);
	}
};

struct NRenderCmdSetPSSamplerStates : public NRenderCmd < NRenderCmdSetPSSamplerStates >
{

	NSamplersState* m_pState;
	UINT32 m_uStartSlot;
	UINT32 m_uNumSamplers;

	inline NRenderCmdSetPSSamplerStates() {}
	inline NRenderCmdSetPSSamplerStates(NSamplersState* state, UINT32 startSlot, UINT32 numSamplers)
		: m_pState(state)
		, m_uStartSlot(startSlot)
		, m_uNumSamplers(numSamplers)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetPSSamplersState(m_pState, m_uStartSlot, m_uNumSamplers);
	}
};

struct NRenderCmdSetCSSamplerStates : public NRenderCmd < NRenderCmdSetCSSamplerStates >
{

	NSamplersState* m_pState;
	UINT32 m_uStartSlot;
	UINT32 m_uNumSamplers;

	inline NRenderCmdSetCSSamplerStates() {}
	inline NRenderCmdSetCSSamplerStates(NSamplersState* state, UINT32 startSlot, UINT32 numSamplers)
		: m_pState(state)
		, m_uStartSlot(startSlot)
		, m_uNumSamplers(numSamplers)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetCSSamplersState(m_pState, m_uStartSlot, m_uNumSamplers);
	}
};

struct NRenderCmdSetVertexShader : public NRenderCmd < NRenderCmdSetVertexShader >
{
	NVertexShader* m_pShader;

	inline NRenderCmdSetVertexShader() {}
	inline NRenderCmdSetVertexShader(NVertexShader* shader)
		: m_pShader(shader)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetVertexShaderWLayout(m_pShader);
	}
};

struct NRenderCmdSetPixelShader : public NRenderCmd < NRenderCmdSetPixelShader >
{
	NPixelShader* m_pShader;

	inline NRenderCmdSetPixelShader() {}
	inline NRenderCmdSetPixelShader(NPixelShader* shader)
		: m_pShader(shader)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetPixelShader(m_pShader);
	}
};

struct NRenderCmdSetGeometryShader : public NRenderCmd < NRenderCmdSetGeometryShader >
{
	NGeometryShader* m_pShader;

	inline NRenderCmdSetGeometryShader() {}
	inline NRenderCmdSetGeometryShader(NGeometryShader* shader)
		: m_pShader(shader)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetGeometryShader(m_pShader);
	}
};

struct NRenderCmdSetComputeShader : public NRenderCmd < NRenderCmdSetComputeShader >
{
	NComputeShader* m_pShader;

	inline NRenderCmdSetComputeShader() {}
	inline NRenderCmdSetComputeShader(NComputeShader* shader)
		: m_pShader(shader)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetComputeShader(m_pShader);
	}
};

struct NRenderCmdSetVertexBuffer : public NRenderCmd < NRenderCmdSetVertexBuffer >
{
	UINT32 m_uStartSlot;
	UINT32 m_uNumBuffers;
	NVertexBuffer* m_pBuffer;

	inline NRenderCmdSetVertexBuffer() {}
	inline NRenderCmdSetVertexBuffer(UINT32 startSlot, UINT32 numBuffers, NVertexBuffer* buffer)
		: m_uStartSlot(startSlot)
		, m_uNumBuffers(numBuffers)
		, m_pBuffer(buffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.BindVertexBuffer(m_uStartSlot, m_uNumBuffers, m_pBuffer);
	}
};

struct NRenderCmdSetIndexBuffer : public NRenderCmd < NRenderCmdSetIndexBuffer >
{
	NIndexBuffer* m_pBuffer;

	inline NRenderCmdSetIndexBuffer() {}
	inline NRenderCmdSetIndexBuffer(NIndexBuffer* buffer)
		: m_pBuffer(buffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.BindIndexBuffer(m_pBuffer);
	}
};

struct NRenderCmdSetPrimitiveTopology : public NRenderCmd < NRenderCmdSetPrimitiveTopology >
{
	UINT32 m_uTopology;

	inline NRenderCmdSetPrimitiveTopology() {}
	inline NRenderCmdSetPrimitiveTopology(UINT32 topology)
		: m_uTopology(topology)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetPrimitiveTopology(m_uTopology);
	}
};

struct NRenderCmdSetConstantBufferData : public NRenderCmd < NRenderCmdSetConstantBufferData >
{
	NConstantBuffer* m_pConstantBuffer;
	void* m_pData;
	SIZE_T m_sSize;

	inline NRenderCmdSetConstantBufferData() {}
	inline NRenderCmdSetConstantBufferData(NConstantBuffer* buffer, void* data, SIZE_T size)
		: m_pConstantBuffer(buffer)
		, m_pData(data)
		, m_sSize(size)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetConstantBufferData(*m_pConstantBuffer, m_pData, m_sSize);
	}
};

struct NRenderCmdSetVSConstantBuffer : public NRenderCmd < NRenderCmdSetVSConstantBuffer >
{
	UINT32 m_uStartSlot;
	UINT32 m_uNumBuffers;
	NConstantBuffer* m_pConstantBuffer;

	inline NRenderCmdSetVSConstantBuffer() {}
	inline NRenderCmdSetVSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
		: m_uStartSlot(startSlot)
		, m_uNumBuffers(numBuffers)
		, m_pConstantBuffer(constantBuffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetVSConstantBuffers(m_uStartSlot, m_uNumBuffers, m_pConstantBuffer);
	}
};

struct NRenderCmdSetPSConstantBuffer : public NRenderCmd < NRenderCmdSetPSConstantBuffer >
{
	UINT32 m_uStartSlot;
	UINT32 m_uNumBuffers;
	NConstantBuffer* m_pConstantBuffer;

	inline NRenderCmdSetPSConstantBuffer() {}
	inline NRenderCmdSetPSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
		: m_uStartSlot(startSlot)
		, m_uNumBuffers(numBuffers)
		, m_pConstantBuffer(constantBuffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetPSConstantBuffers(m_uStartSlot, m_uNumBuffers, m_pConstantBuffer);
	}
};

struct NRenderCmdSetGSConstantBuffer : public NRenderCmd < NRenderCmdSetGSConstantBuffer >
{
	UINT32 m_uStartSlot;
	UINT32 m_uNumBuffers;
	NConstantBuffer* m_pConstantBuffer;

	inline NRenderCmdSetGSConstantBuffer() {}
	inline NRenderCmdSetGSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
		: m_uStartSlot(startSlot)
		, m_uNumBuffers(numBuffers)
		, m_pConstantBuffer(constantBuffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetGSConstantBuffers(m_uStartSlot, m_uNumBuffers, m_pConstantBuffer);
	}
};

struct NRenderCmdSetCSConstantBuffer : public NRenderCmd < NRenderCmdSetCSConstantBuffer >
{
	UINT32 m_uStartSlot;
	UINT32 m_uNumBuffers;
	NConstantBuffer* m_pConstantBuffer;

	inline NRenderCmdSetCSConstantBuffer() {}
	inline NRenderCmdSetCSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
		: m_uStartSlot(startSlot)
		, m_uNumBuffers(numBuffers)
		, m_pConstantBuffer(constantBuffer)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.SetCSConstantBuffers(m_uStartSlot, m_uNumBuffers, m_pConstantBuffer);
	}
};

struct NRenderCmdSetVSTextureResource : public NRenderCmd < NRenderCmdSetVSTextureResource >
{
	UINT32 m_uStartSlot;
	BOOL m_bUseTextures;
	UINT32 m_u32NumSlot;
	NDX11Texture2DRes* m_pTextures[N_G_MAX_TEXTURERESOURCEVIEWS];
	NDX11StructureBuffer* m_pBuffers[N_G_MAX_TEXTURERESOURCEVIEWS];

	inline NRenderCmdSetVSTextureResource() {}
	inline NRenderCmdSetVSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11Texture2DRes** textures)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(true)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pTextures[i] = textures[i];
	}
	inline NRenderCmdSetVSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11StructureBuffer** buffers)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(false)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pBuffers[i] = buffers[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		if (!m_bUseTextures)
			Dev.SetVSStructureResources(m_uStartSlot, m_u32NumSlot, m_pBuffers);
		else
			Dev.SetVSTextureResources(m_uStartSlot, m_u32NumSlot, m_pTextures);
	}
};

struct NRenderCmdSetPSTextureResource : public NRenderCmd < NRenderCmdSetPSTextureResource >
{
	UINT32 m_uStartSlot;
	BOOL m_bUseTextures;
	UINT32 m_u32NumSlot;
	NDX11Texture2DRes* m_pTextures[N_G_MAX_TEXTURERESOURCEVIEWS];
	NDX11StructureBuffer* m_pBuffers[N_G_MAX_TEXTURERESOURCEVIEWS];

	inline NRenderCmdSetPSTextureResource() {}
	inline NRenderCmdSetPSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11Texture2DRes** textures)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(true)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pTextures[i] = textures[i];
	}
	inline NRenderCmdSetPSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11StructureBuffer** buffers)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(false)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pBuffers[i] = buffers[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		if (!m_bUseTextures)
			Dev.SetPSStructureResources(m_uStartSlot, m_u32NumSlot, m_pBuffers);
		else
			Dev.SetPSTextureResources(m_uStartSlot, m_u32NumSlot, m_pTextures);
	}
};

struct NRenderCmdSetGSTextureResource : public NRenderCmd < NRenderCmdSetGSTextureResource >
{
	UINT32 m_uStartSlot;
	BOOL m_bUseTextures;
	UINT32 m_u32NumSlot;
	NDX11Texture2DRes* m_pTextures[N_G_MAX_TEXTURERESOURCEVIEWS];
	NDX11StructureBuffer* m_pBuffers[N_G_MAX_TEXTURERESOURCEVIEWS];

	inline NRenderCmdSetGSTextureResource() {}
	inline NRenderCmdSetGSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11Texture2DRes** textures)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(true)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pTextures[i] = textures[i];
	}
	inline NRenderCmdSetGSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11StructureBuffer** buffers)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(false)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pBuffers[i] = buffers[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		if (!m_bUseTextures)
			Dev.SetGSStructureResources(m_uStartSlot, m_u32NumSlot, m_pBuffers);
		else
			Dev.SetGSTextureResources(m_uStartSlot, m_u32NumSlot, m_pTextures);
	}
};

struct NRenderCmdSetCSTextureResource : public NRenderCmd < NRenderCmdSetCSTextureResource >
{
	UINT32 m_uStartSlot;
	BOOL m_bUseTextures;
	UINT32 m_u32NumSlot;
	NDX11Texture2DRes* m_pTextures[N_G_MAX_TEXTURERESOURCEVIEWS];
	NDX11StructureBuffer* m_pBuffers[N_G_MAX_TEXTURERESOURCEVIEWS];

	inline NRenderCmdSetCSTextureResource() {}
	inline NRenderCmdSetCSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11Texture2DRes** textures)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(true)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pTextures[i] = textures[i];
	}
	inline NRenderCmdSetCSTextureResource(UINT32 startSlot, UINT32 numSlot, NDX11StructureBuffer** buffers)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(false)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pBuffers[i] = buffers[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		if (!m_bUseTextures)
			Dev.SetCSStructureResources(m_uStartSlot, m_u32NumSlot, m_pBuffers);
		else
			Dev.SetCSTextureResources(m_uStartSlot, m_u32NumSlot, m_pTextures);
	}
};

struct NRenderCmdSetCSUnorderedAccessView : public NRenderCmd < NRenderCmdSetCSUnorderedAccessView >
{
	UINT32 m_uStartSlot;
	BOOL m_bUseTextures;
	UINT32 m_u32NumSlot;
	NDX11Texture2DRes* m_pTextures[N_G_MAX_UNORDEREDRESOURCEVIEWS];
	NDX11StructureBuffer* m_pBuffers[N_G_MAX_UNORDEREDRESOURCEVIEWS];

	inline NRenderCmdSetCSUnorderedAccessView() {}
	inline NRenderCmdSetCSUnorderedAccessView(UINT32 startSlot, UINT32 numSlot, NDX11Texture2DRes** textures)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(true)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pTextures[i] = textures[i];
	}
	inline NRenderCmdSetCSUnorderedAccessView(UINT32 startSlot, UINT32 numSlot, NDX11StructureBuffer** buffers)
		: m_uStartSlot(startSlot)
		, m_bUseTextures(false)
		, m_u32NumSlot(numSlot)
	{
		for (UINT32 i = 0; i < numSlot; i++)
			m_pBuffers[i] = buffers[i];
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		if (!m_bUseTextures)
			Dev.SetCSUnoderedAccessViews(m_uStartSlot, m_u32NumSlot, m_pBuffers);
		else
			Dev.SetCSUnoderedAccessViews(m_uStartSlot, m_u32NumSlot, m_pTextures);
	}
};

struct NRenderCmdDrawIndexed : public NRenderCmd < NRenderCmdDrawIndexed >
{
	UINT32 m_uIndexCount;
	UINT32 m_uIndexOffset;
	UINT32 m_uVertexOffset;

	inline NRenderCmdDrawIndexed() {}
	inline NRenderCmdDrawIndexed(UINT32 indexCount, UINT32 indexOffset, UINT32 vertexOffset)
		: m_uIndexCount(indexCount)
		, m_uIndexOffset(indexOffset)
		, m_uVertexOffset(vertexOffset)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.DrawIndexed(m_uIndexCount, m_uIndexOffset, m_uVertexOffset);
	}
};

struct NRenderCmdDraw : public NRenderCmd < NRenderCmdDraw >
{
	UINT32 m_uVertexCount;
	UINT32 m_uVertexOffset;

	inline NRenderCmdDraw() {}
	inline NRenderCmdDraw(UINT32 vertexCount, UINT32 vertexOffset)
		: m_uVertexCount(vertexCount)
		, m_uVertexOffset(vertexOffset)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.Draw(m_uVertexCount, m_uVertexOffset);
	}
};

struct NRenderCmdDispatch : public NRenderCmd < NRenderCmdDispatch >
{
	UINT32 m_uThreadGroupX;
	UINT32 m_uThreadGroupY;
	UINT32 m_uThreadGroupZ;

	inline NRenderCmdDispatch() {}
	inline NRenderCmdDispatch(UINT32 threadGroupX, UINT32 threadGroupY, UINT32 threadGroupZ)
		: m_uThreadGroupX(threadGroupX)
		, m_uThreadGroupY(threadGroupY)
		, m_uThreadGroupZ(threadGroupZ)
	{
	}


	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.Dispatch(m_uThreadGroupX, m_uThreadGroupY, m_uThreadGroupZ);
	}
};

struct NRenderCmdWriteTextureToDSS : public NRenderCmd < NRenderCmdWriteTextureToDSS >
{
	NTexture2DRes* m_pTexture;
	std::string m_sFilepath;
	N_RESOURCE_FORMAT m_eFormat;

	inline NRenderCmdWriteTextureToDSS() {}
	inline NRenderCmdWriteTextureToDSS(NTexture2DRes* texture, std::string filepath, N_RESOURCE_FORMAT format)
		: m_pTexture(texture)
		, m_sFilepath(filepath)
		, m_eFormat(format)
	{
	}

	void Execute(NGraphicsDevice& Dev, NRenderCmdListBase& CmdList)
	{
		Dev.WriteTextureToDDS(*m_pTexture, m_sFilepath, m_eFormat);
	}
};


// =============================================
// Construction Area - End
// =============================================

class NRenderCmdList : public NRenderCmdListBase
{
public:
	inline void CmdClear(UINT32 mask = N_B_ALL
		, NTexture2DRes* renderTarget = nullptr
		, NTexture2DRes* depthStencilTarget = nullptr
		, float depth = -1.f, UINT32 stencil = 0
		, float colorR = -1.f, float colorG = -1.f, float colorB = -1.f, float colorA = -1.f)
	{
		new (AllocCommand<NRenderCmdClear>()) NRenderCmdClear(mask, renderTarget, depthStencilTarget, depth, stencil, colorR, colorG, colorB, colorA);
	}

	inline void CmdPresent()
	{
		new (AllocCommand<NRenderCmdPresent>()) NRenderCmdPresent();
	}

	inline void CmdSetViewports(UINT32 viewportsNum, N_G_VIEWPORT viewports[N_G_MAX_VIEWPORTS])
	{
		new (AllocCommand<NRenderCmdSetViewports>()) NRenderCmdSetViewports(viewportsNum, viewports);
	}

	inline void CmdSetDefViewports()
	{
		new (AllocCommand<NRenderCmdSetDefViewports>()) NRenderCmdSetDefViewports();
	}

	inline void CmdSetRendertargets(UINT32 viewsnum, NTexture2DRes* rendertargets[N_G_MAX_RENDERTARGETVIEWS], NTexture2DRes* depthstencil)
	{
		new (AllocCommand<NRenderCmdSetRendertargets>()) NRenderCmdSetRendertargets(viewsnum, rendertargets, depthstencil);
	}

	inline void CmdSetRasterizerState(NRasterizerState* state)
	{
		new (AllocCommand<NRenderCmdSetRasterizerState>()) NRenderCmdSetRasterizerState(state);
	}

	inline void CmdSetDepthStencilState(NDepthStencilState* state, UINT32 stencilref)
	{
		new (AllocCommand<NRenderCmdSetDepthStencilState>()) NRenderCmdSetDepthStencilState(state, stencilref);
	}

	inline void CmdSetBlendState(NBlendState* state, float blendFactors[4], UINT32 sampleMask)
	{
		new (AllocCommand<NRenderCmdSetBlendState>()) NRenderCmdSetBlendState(state, blendFactors, sampleMask);
	}

	inline void CmdSetPSSamplersState(NSamplersState* state, UINT32 startSlot, UINT32 numSamplers)
	{
		new (AllocCommand<NRenderCmdSetPSSamplerStates>()) NRenderCmdSetPSSamplerStates(state, startSlot, numSamplers);
	}

	inline void CmdSetCSSamplersState(NSamplersState* state, UINT32 startSlot, UINT32 numSamplers)
	{
		new (AllocCommand<NRenderCmdSetCSSamplerStates>()) NRenderCmdSetCSSamplerStates(state, startSlot, numSamplers);
	}

	inline void CmdSetVertexShader(NVertexShader* shader)
	{
		new (AllocCommand<NRenderCmdSetVertexShader>()) NRenderCmdSetVertexShader(shader);
	}

	inline void CmdSetPixelShader(NPixelShader* shader)
	{
		new (AllocCommand<NRenderCmdSetPixelShader>()) NRenderCmdSetPixelShader(shader);
	}

	inline void CmdSetGeometryShader(NGeometryShader* shader)
	{
		new (AllocCommand<NRenderCmdSetGeometryShader>()) NRenderCmdSetGeometryShader(shader);
	}

	inline void CmdSetComputeShader(NComputeShader* shader)
	{
		new (AllocCommand<NRenderCmdSetComputeShader>()) NRenderCmdSetComputeShader(shader);
	}

	inline void CmdSetVertexBuffer(UINT32 startSlot, UINT32 numBuffers, NVertexBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetVertexBuffer>()) NRenderCmdSetVertexBuffer(startSlot, numBuffers, buffer);
	}

	inline void CmdSetIndexBuffer(NIndexBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetIndexBuffer>()) NRenderCmdSetIndexBuffer(buffer);
	}

	inline void CmdSetPrimitiveTopology(UINT32 topology)
	{
		new (AllocCommand<NRenderCmdSetPrimitiveTopology>()) NRenderCmdSetPrimitiveTopology(topology);
	}

	inline void CmdSetConstantBufferData(NConstantBuffer* buffer, void* data, SIZE_T size)
	{
		new (AllocCommand<NRenderCmdSetConstantBufferData>()) NRenderCmdSetConstantBufferData(buffer, data, size);
	}

	inline void CmdSetVSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
	{
		new (AllocCommand<NRenderCmdSetVSConstantBuffer>()) NRenderCmdSetVSConstantBuffer(startSlot, numBuffers, constantBuffer);
	}

	inline void CmdSetPSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
	{
		new (AllocCommand<NRenderCmdSetPSConstantBuffer>()) NRenderCmdSetPSConstantBuffer(startSlot, numBuffers, constantBuffer);
	}

	inline void CmdSetGSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
	{
		new (AllocCommand<NRenderCmdSetGSConstantBuffer>()) NRenderCmdSetGSConstantBuffer(startSlot, numBuffers, constantBuffer);
	}

	inline void CmdSetCSConstantBuffer(UINT32 startSlot, UINT32 numBuffers, NConstantBuffer* constantBuffer)
	{
		new (AllocCommand<NRenderCmdSetCSConstantBuffer>()) NRenderCmdSetCSConstantBuffer(startSlot, numBuffers, constantBuffer);
	}

	inline void CmdSetVSTextureResource(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetVSTextureResource>()) NRenderCmdSetVSTextureResource(startSlot, 1, &texture);
	}

	inline void CmdSetPSTextureResource(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetPSTextureResource>()) NRenderCmdSetPSTextureResource(startSlot, 1, &texture);
	}

	inline void CmdSetGSTextureResource(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetGSTextureResource>()) NRenderCmdSetGSTextureResource(startSlot, 1, &texture);
	}

	inline void CmdSetCSTextureResource(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetCSTextureResource>()) NRenderCmdSetCSTextureResource(startSlot, 1, &texture);
	}

	inline void CmdSetVSStructureResource(UINT32 startSlot, NDX11StructureBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetVSTextureResource>()) NRenderCmdSetVSTextureResource(startSlot, 1, &buffer);
	}

	inline void CmdSetPSStructureResource(UINT32 startSlot, NDX11StructureBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetPSTextureResource>()) NRenderCmdSetPSTextureResource(startSlot, 1, &buffer);
	}

	inline void CmdSetGSStructureResource(UINT32 startSlot, NDX11StructureBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetGSTextureResource>()) NRenderCmdSetGSTextureResource(startSlot, 1, &buffer);
	}

	inline void CmdSetCSStructureResource(UINT32 startSlot, NDX11StructureBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetCSTextureResource>()) NRenderCmdSetCSTextureResource(startSlot, 1, &buffer);
	}

	inline void CmdSetCSStructureResource(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetCSTextureResource>()) NRenderCmdSetCSTextureResource(startSlot, 1, &texture);
	}

	inline void CmdSetCSTextureUnorderedAccessView(UINT32 startSlot, NDX11Texture2DRes* texture)
	{
		new (AllocCommand<NRenderCmdSetCSUnorderedAccessView>()) NRenderCmdSetCSUnorderedAccessView(startSlot, 1, &texture);
	}

	inline void CmdSetCSStructureUnorderedAccessView(UINT32 startSlot, NDX11StructureBuffer* buffer)
	{
		new (AllocCommand<NRenderCmdSetCSUnorderedAccessView>()) NRenderCmdSetCSUnorderedAccessView(startSlot, 1, &buffer);
	}

	inline void CmdDrawIndexed(UINT32 indexCount, UINT32 indexOffset, UINT32 vertexOffset)
	{
		new (AllocCommand<NRenderCmdDrawIndexed>()) NRenderCmdDrawIndexed(indexCount, indexOffset, vertexOffset);
	}

	inline void CmdDraw(UINT32 vertexCount, UINT32 vertexOffset)
	{
		new (AllocCommand<NRenderCmdDraw>()) NRenderCmdDraw(vertexCount, vertexOffset);
	}

	inline void CmdDispatch(UINT32 threadGroupX, UINT32 threadGroupY, UINT32 threadGroupZ)
	{
		new (AllocCommand<NRenderCmdDispatch>()) NRenderCmdDispatch(threadGroupX, threadGroupY, threadGroupZ);
	}

	inline void CmdWriteTextureToDDS(NTexture2DRes* texture, std::string filepath, N_RESOURCE_FORMAT format = N_R_FORMAT_UNKNOWN)
	{
		new (AllocCommand<NRenderCmdWriteTextureToDSS>()) NRenderCmdWriteTextureToDSS(texture, filepath, format);
	}
};