#pragma once

#include "../Macro/Macro.h"
#include "../Template/NNonCopyable.h"

#include <Windows.h>
#include <string>
#include <queue>

class NWin32 : public NNonCopyable
{
public:
	NWin32(HINSTANCE hInstance);
	
	BOOL CreateMainWindow();
	INT32 Run();

	virtual BOOL VInit();
    virtual void VOnUpdate() {}
	virtual void VOnActive(BOOL active);
	virtual void VOnResize(UINT32 width, UINT32 height, UINT wParam);

	// Temporary be here @TODO might send an event indicate resizing to tell other component instead of this
	// (Need event system)
	virtual void VOnRendererResize() {} 

	virtual void VOnEnterSizeMove();
	virtual void VOnExitSizeMove();
	virtual void VOnDestroy();

	struct MsgProcResult
	{
		BOOL processed;
		LRESULT result;
		MsgProcResult(BOOL isProcessed, LRESULT r) : processed(isProcessed), result(r) {}
	};
	virtual MsgProcResult VMsgBufferProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HINSTANCE	GetHInstant() const	{ return m_hAppInst; }
	HWND		GetHWND() const		{ return m_hMainWnd; }

    std::wstring GetMainWindowCaption() const { return m_sMainWndCaption; }
	UINT32		GetClientWidth() const	{ return m_uClientWidth;  }
	UINT32		GetClientHeight() const	{ return m_uClientHeight; }
	float		GetAspectRatio() const { return static_cast<float>(m_uClientWidth) / m_uClientHeight; } 
    BOOL        GetActivated() const { return m_bActivated; }
    UINT32      GetQuitParam() const { return m_uQuitParam; }
	

protected:
	HINSTANCE	m_hAppInst;
	HWND		m_hMainWnd;

	BOOL		m_bActivated;
	BOOL		m_bMinimized;
	BOOL		m_bMaximized;
	BOOL		m_bResizing;

	UINT32		m_uClientWidth;
	UINT32		m_uClientHeight;

	std::wstring m_sMainWndCaption;

    UINT32      m_uQuitParam;

private:
	// Only one window. If need more than one -> use list of pointers instead of a pointer here.
	static NWin32* sCurrentWin32;

};