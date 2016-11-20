#include "NRenderCmd.h"

//NRenderCmd::PFnSubmit NRenderCmd::g_CommandTable[] =
//{
//    &Submit_ClearBuffers,
//    &Submit_ClearRenderTarget,
//    &Submit_ClearDepthStencil,
//    &Submit_Present,
//    &Submit_SetViewPorts,
//    &Submit_SetRenderTargets,
//    &Submit_SetRasterizerState,
//    &Submit_SetDepthStencilState,
//    &Submit_SetBlendState,
//};
//
//NRenderCmd::PFnDelete NRenderCmd::g_CommandDeleteTable[] =
//{
//	&Delete_ClearBuffers,
//	&Delete_ClearRenderTarget,
//	&Delete_ClearDepthStencil,
//	&Delete_Present,
//	&Delete_SetViewPorts,
//	&Delete_SetRenderTargets,
//	&Delete_SetRasterizerState,
//	&Delete_SetDepthStencilState,
//	&Delete_SetBlendState,
//};
//
//void NRenderCmd::Submit_ClearBuffers(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::CLEAR_BUFFERS);
//    CMD_CLEAR_BUFFERS& data = *(CMD_CLEAR_BUFFERS *)&cmd;
//
//	dev.Clear(data.u32ClearMask);
//
//	//N_DELETE((CMD_CLEAR_BUFFERS *)&cmd);
//}
//
//void NRenderCmd::Submit_ClearRenderTarget(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::CLEAR_RENDERTARGET);
//    CMD_CLEAR_RENDERTARGET& data = *(CMD_CLEAR_RENDERTARGET *)&cmd;
//    
//	dev.ClearRenderTargetView(data.pRenderTarget->GetRTV(), data.fColor);
//
//	//N_DELETE((CMD_CLEAR_RENDERTARGET *)&cmd);
//}
//
//void NRenderCmd::Submit_ClearDepthStencil(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::CLEAR_DEPTHSTENCIL);
//    CMD_CLEAR_DEPTHSTENCIL& data = *(CMD_CLEAR_DEPTHSTENCIL *)&cmd;
//    
//	dev.ClearDepthStencilView(data.pDepthStencil->GetDSV(), data.u32ClearFlags, data.fDepth, data.u8Stencil);
//
//	//N_DELETE((CMD_CLEAR_DEPTHSTENCIL *)&cmd);
//}
//
//void NRenderCmd::Submit_Present(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::PRESENT);
//    CMD_PRESENT& data = *(CMD_PRESENT *)&cmd;
//    
//	dev.Present();
//
//	//N_DELETE((CMD_PRESENT *)&cmd);
//}
//
//void NRenderCmd::Submit_SetViewPorts(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::SET_VIEWPORTS);
//    CMD_SET_VIEWPORTS& data = *(CMD_SET_VIEWPORTS *)&cmd;
//    
//	dev.SetViewports(data.pViewports, data.u32ViewPortNum);
//
//	//N_DELETE((CMD_SET_VIEWPORTS *)&cmd);
//}
//
//void NRenderCmd::Submit_SetRenderTargets(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::SET_RENDERTARGETS);
//    CMD_SET_RENDERTARGETS& data = *(CMD_SET_RENDERTARGETS *)&cmd;
//    
//	dev.GetRenderTargetManager()->VActiveRenderTargets(data.pRenderTargets, data.pDepthStencil, data.u32ViewNum);
//
//	//N_DELETE((CMD_SET_RENDERTARGETS *)&cmd);
//}
//
//void NRenderCmd::Submit_SetRasterizerState(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::SET_RASTERIZERSTATE);
//    CMD_SET_RASTERIZERSTATE& data = *(CMD_SET_RASTERIZERSTATE *)&cmd;
//    
//	data.pState->VActive();
//
//	//N_DELETE((CMD_SET_RASTERIZERSTATE *)&cmd);
//}
//
//void NRenderCmd::Submit_SetDepthStencilState(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::SET_DEPTHSTENCILSTATE);
//    CMD_SET_DEPTHSTENCILSTATE& data = *(CMD_SET_DEPTHSTENCILSTATE *)&cmd;
//    
//	data.pState->SetStencilRef(data.u32StencilRef);
//	data.pState->VActive();
//
//	//N_DELETE((CMD_SET_DEPTHSTENCILSTATE *)&cmd);
//}
//
//void NRenderCmd::Submit_SetBlendState(NGraphicsDevice& dev, COMMAND& cmd)
//{
//    N_ASSERT(cmd.uId == COMMAND_TYPE::SET_BLENDSTATE);
//    CMD_SET_BLENDSTATE& data = *(CMD_SET_BLENDSTATE *)&cmd;
//    
//	data.pState->SetBlend(data.fBlendFactors, data.u32SampleMask);
//	data.pState->VActive();
//
//	//N_DELETE((CMD_SET_BLENDSTATE *)&cmd);
//}



void NRenderCmdListBase::Reset()
{
	m_pRoot = m_pCommandLink = nullptr;
	m_uNumCommands = 0;
}