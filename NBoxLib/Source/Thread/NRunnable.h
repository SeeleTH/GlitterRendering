#pragma once
#include "../Macro/Macro.h"

#include <Windows.h>

class NRunnable
{
public:
    // Other prioities can be:
    // THREAD_PRIORITY_ABOVE_NORMAL
    // THREAD_PRIORITY_BELOW_NORMAL
    // THREAD_PRIORITY_HIGHEST
    // THREAD_PRIORITY_TIME_CRITICAL
    // THREAD_PRIORITY_LOWEST
    // THREAD_PRIORITY_IDLE
    //
    NRunnable(int priority = THREAD_PRIORITY_NORMAL)
    {
        m_dThreadID = 0;
        m_iThreadPriority = priority;
    }

    virtual ~NRunnable() {
        if (m_hThread != 0) {
            CloseHandle(m_hThread);
            VOnExit();
        }
    }

    BOOL Init();
    BOOL Run();
    BOOL Stop();
    BOOL Exit(DWORD dwExitCode);
	DWORD Wait(DWORD millisec = INFINITE);

    static DWORD WINAPI ThreadProc(LPVOID lpParam);

protected:
    virtual BOOL VOnInit() { return true; }
    virtual void VThreadProc(void) = 0;
    virtual void VOnExit() {}

    HANDLE m_hThread;
    DWORD m_dThreadID;
    INT32 m_iThreadPriority;
};