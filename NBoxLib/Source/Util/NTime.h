#pragma once
#include "../Macro/Macro.h"

class NTime
{
public:
    NTime();

    UINT64   GetRealTime();

    void Update(BOOL bUpdateVirtual);
    void UpdateBy(UINT64 u64Ticks, BOOL bUpdateVirtual);

    UINT64 GetCurMicros() const { return m_u64CurMicros; }
    UINT64 GetDeltaMicros() const { return m_u64DeltaMicros; }
    float GetDeltaFloat() const { return m_f32DeltaFloat; }

protected:
    double   m_dSecondsPerCount;
    UINT64   m_u64CurTime;
    UINT64   m_u64LastTime;
    UINT64   m_u64LastRealTime;

    UINT64   m_u64CurMicros;
    UINT64   m_u64DeltaMicros;
    float    m_f32DeltaFloat;
};