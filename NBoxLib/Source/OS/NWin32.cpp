#include "NWin32.h"

NWin32* NWin32::sCurrentWin32 = NULL;

NWin32::NWin32(HINSTANCE hInstance)
	: m_hAppInst(hInstance)
	, m_hMainWnd(0)
	, m_bActivated(false)
	, m_bMinimized(false)
	, m_bMaximized(false)
	, m_bResizing(false)
	, m_uClientWidth(1280)//(1600)
	, m_uClientHeight(720)//(900)
	, m_sMainWndCaption(L"NBoxLib's Unnamed Wnd")
{
	sCurrentWin32 = this;
}

	
BOOL NWin32::CreateMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MsgProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"NBoxLibWndClassName";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_uClientWidth, m_uClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	INT32 width  = R.right - R.left;
	INT32 height = R.bottom - R.top;

	m_hMainWnd = CreateWindow(L"NBoxLibWndClassName", m_sMainWndCaption.c_str(), 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0); 
	if( !m_hMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

INT32 NWin32::Run()
{
	MSG msg = {0};
	if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
	{
        TranslateMessage( &msg );
        DispatchMessage( &msg );
	}

    VOnUpdate();

    if (msg.message == WM_QUIT)
        m_uQuitParam = (UINT32)msg.wParam;

	return (msg.message != WM_QUIT);
}

BOOL NWin32::VInit()
{
	return CreateMainWindow();
}

void NWin32::VOnActive(BOOL active)
{
	m_bActivated = active;
}

void NWin32::VOnResize(UINT32 width, UINT32 height, UINT wParam)
{
	if( wParam == SIZE_MINIMIZED )
	{
		m_bActivated = false;
		m_bMinimized = true;
		m_bMaximized = false;
		VOnRendererResize();
	}
	else if( wParam == SIZE_MAXIMIZED )
	{
		m_bActivated = true;
		m_bMinimized = false;
		m_bMaximized = true;
		VOnRendererResize();
	}
	else if( wParam == SIZE_RESTORED )
	{

		// Restoring from minimized state?
		if( m_bMinimized )
		{
			m_bActivated = true;
			m_bMinimized = false;
			VOnRendererResize();
		}

		// Restoring from maximized state?
		else if( m_bMaximized )
		{
			m_bActivated = false;
			m_bMaximized = false;
			VOnRendererResize();
		}

		else if(m_bResizing)
		{
		}

		else
		{
			VOnRendererResize();
		}
	}
}

void NWin32::VOnEnterSizeMove()
{
	m_bActivated = false;
	m_bResizing  = true;
}

void NWin32::VOnExitSizeMove()
{
	m_bActivated = true;
	m_bResizing  = false;
	VOnRendererResize();
}

void NWin32::VOnDestroy()
{
	PostQuitMessage(0);
}

NWin32::MsgProcResult NWin32::VMsgBufferProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Input buffer check message if isn't related then put msg in buffer for further use.

	// @TODO keyboard and mouse buffer

	switch( msg )
	{
		case WM_ACTIVATE:
			VOnActive(( LOWORD(wParam) != WA_INACTIVE ));
			return MsgProcResult(true,0);

		case WM_SIZE:
			VOnResize(LOWORD(lParam), HIWORD(lParam), wParam);
			return MsgProcResult(true,0);

		case WM_ENTERSIZEMOVE:
			VOnEnterSizeMove();
			return MsgProcResult(true,0);
			
		case WM_EXITSIZEMOVE:
			VOnExitSizeMove();
			return MsgProcResult(true,0);
			
		case WM_DESTROY:
			VOnDestroy();
			return MsgProcResult(true,0);

		case WM_MENUCHAR:
			return MsgProcResult(true,MAKELRESULT(0, MNC_CLOSE));

		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
			return MsgProcResult(true,0);
	}

	return MsgProcResult(false,0);
}
	
LRESULT CALLBACK NWin32::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MsgProcResult result(false, 0);

	if(sCurrentWin32)
		result = sCurrentWin32->VMsgBufferProc(hwnd, msg, wParam, lParam);

	if(result.processed)
		return result.result;

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
