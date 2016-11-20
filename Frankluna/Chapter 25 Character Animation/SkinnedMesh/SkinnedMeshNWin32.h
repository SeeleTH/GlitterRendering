#pragma once

#include "../../../NBoxLib/Source/OS/NWin32.h"

// Temporary Class in order to test NWin32 class in practice 
// (and iteratively change SkinnedMesh code base)
class SkinnedMeshNWin32 : public NWin32
{
public:
	SkinnedMeshNWin32(HINSTANCE hInstance);

	virtual MsgProcResult VMsgBufferProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    virtual void VOnUpdate();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	// Temporary mouse related struct, function and member variable 
	// in order to make it compatible with current skinmesh codebase.
	struct TempMouseBehaviorInfo
	{
		BOOL m_bIsDowned;
		POINT m_pLastMousePos;
		float m_fDX;
		float m_fDY;
		TempMouseBehaviorInfo() : m_bIsDowned(false), m_fDX(0.f), m_fDY(0.f) {}
	};
	const TempMouseBehaviorInfo* GetMouseInfo() { return &m_sMouse; }

protected:
    TempMouseBehaviorInfo m_sMouse;
    TempMouseBehaviorInfo m_sMouseBuffer;
};