#pragma once

#include "../Macro/Macro.h"
#include "../Template/NNonCopyable.h"

#include <Windows.h>

class NCriticalSection : public NNonCopyable
{
public:
    NCriticalSection() { InitializeCriticalSection(&m_cs); }
    ~NCriticalSection() { DeleteCriticalSection(&m_cs); }
    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

protected:
    mutable CRITICAL_SECTION m_cs;
};

class NScopedCriticalSection : public NNonCopyable
{
public:
    NScopedCriticalSection(NCriticalSection& csResource) : m_csResource(csResource)
    {
        m_csResource.Lock();
    }

    ~NScopedCriticalSection()
    {
        m_csResource.Unlock();
    }

protected:
    NCriticalSection &m_csResource;
};