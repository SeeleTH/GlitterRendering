#include "NTime.h"

#include <Windows.h>

NTime::NTime()
: m_u64CurTime(0ULL)
, m_u64LastTime(0ULL)
, m_u64CurMicros(0ULL)
, m_u64DeltaMicros(0ULL)
, m_f32DeltaFloat(0.f)
{
    __int64 countsPerSec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
    m_dSecondsPerCount = 1.0 / (double)countsPerSec;
    m_u64LastRealTime = GetRealTime();
}

UINT64 NTime::GetRealTime()
{
    UINT64 realtime;
    QueryPerformanceCounter((LARGE_INTEGER*)&realtime);
    return realtime;
}

void NTime::Update(BOOL bUpdateVirtual)
{
    UINT64 u64TimeNow = GetRealTime();
    UINT64 u64Delta = u64TimeNow - m_u64LastRealTime;
    m_u64LastRealTime = u64TimeNow;

    UpdateBy(u64Delta, bUpdateVirtual);
}

void NTime::UpdateBy(UINT64 u64Ticks, BOOL bUpdateVirtual)
{
    m_u64LastTime = m_u64CurTime;
    m_u64CurTime += u64Ticks;

    UINT64 u64LastMicros = m_u64CurMicros;
    m_u64CurMicros = static_cast<UINT64>(m_u64CurTime * (m_dSecondsPerCount * 1000000ULL));
    m_u64DeltaMicros = m_u64CurMicros - u64LastMicros;
    m_f32DeltaFloat = m_u64DeltaMicros * static_cast<float>(1.0 / 1000000.0);
}