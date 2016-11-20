#include "NRunnable.h"

BOOL NRunnable::Init(void)
{
    m_hThread = CreateThread(
        NULL, // default security attributes
        0, // default stack size
        ThreadProc, // thread process
        this, // thread parameter is a pointer to the process
        CREATE_SUSPENDED, // default creation flags
        &m_dThreadID); // receive thread identifier
    if (m_hThread == NULL)
    {
        return false;
    }
    SetThreadPriority(m_hThread, m_iThreadPriority);

    return VOnInit();
}

BOOL NRunnable::Run()
{
    return ResumeThread(m_hThread) >= 0;
}

BOOL NRunnable::Stop()
{
    return SuspendThread(m_hThread) >= 0;
}

BOOL NRunnable::Exit(DWORD dwExitCode)
{
    if (TerminateThread(m_hThread, dwExitCode))
    {
        CloseHandle(m_hThread);
        VOnExit();
        m_hThread = 0;
        return true;
    }
    return false;
}

DWORD  NRunnable::Wait(DWORD millisec)
{
	return WaitForSingleObject(m_hThread, millisec);
}

DWORD WINAPI NRunnable::ThreadProc(LPVOID lpParam)
{
    NRunnable *proc = static_cast<NRunnable *>(lpParam);
    proc->VThreadProc();
    return true;
}