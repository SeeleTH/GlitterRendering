#include "SkinnedMeshNWin32.h"

#include <xnamath.h>

SkinnedMeshNWin32::SkinnedMeshNWin32(HINSTANCE hInstance)
	:NWin32(hInstance)
{
}

SkinnedMeshNWin32::MsgProcResult SkinnedMeshNWin32::VMsgBufferProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Temporary mouse related things
	switch( msg )
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		return MsgProcResult(true, 0);
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		return MsgProcResult(true, 0);
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		return MsgProcResult(true, 0);
	}

	return NWin32::VMsgBufferProc(hwnd, msg, wParam, lParam);
}


void SkinnedMeshNWin32::VOnUpdate()
{
    // Update mouse
    m_sMouse.m_bIsDowned = m_sMouseBuffer.m_bIsDowned;
    m_sMouse.m_fDX = XMConvertToRadians(0.25f*static_cast<float>(m_sMouseBuffer.m_pLastMousePos.x - m_sMouse.m_pLastMousePos.x));
    m_sMouse.m_fDY = XMConvertToRadians(0.25f*static_cast<float>(m_sMouseBuffer.m_pLastMousePos.y - m_sMouse.m_pLastMousePos.y));
    m_sMouse.m_pLastMousePos = m_sMouseBuffer.m_pLastMousePos;
}

void SkinnedMeshNWin32::OnMouseDown(WPARAM btnState, int x, int y)
{
    m_sMouseBuffer.m_pLastMousePos.x = x;
    m_sMouseBuffer.m_pLastMousePos.y = y;
    m_sMouseBuffer.m_bIsDowned = true;

	SetCapture(m_hMainWnd);
}

void SkinnedMeshNWin32::OnMouseUp(WPARAM btnState, int x, int y)
{
    m_sMouseBuffer.m_bIsDowned = false;

	ReleaseCapture();
}

void SkinnedMeshNWin32::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
        //m_sMouseBuffer.m_fDX = XMConvertToRadians(0.25f*static_cast<float>(x - m_sMouse.m_pLastMousePos.x));
        //m_sMouseBuffer.m_fDY = XMConvertToRadians(0.25f*static_cast<float>(y - m_sMouse.m_pLastMousePos.y));
	}
    m_sMouseBuffer.m_pLastMousePos.x = x;
    m_sMouseBuffer.m_pLastMousePos.y = y;
}