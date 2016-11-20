#pragma once

#include "../Macro/Macro.h"
#include "NCriticalSection.h"
#include <queue>

template<typename Data>
class NConcurrentQueue
{
private:
    std::queue<Data> m_queue;
    NCriticalSection m_cs;
    HANDLE m_dataPushed;
public:
    NConcurrentQueue() { m_dataPushed = CreateEvent(NULL, true, false, NULL); }
    void push(Data const& data)
    {
        {
            NScopedCriticalSection locker(m_cs);
            m_queue.push(data);
        }
        PulseEvent(m_dataPushed);
    }

    BOOL empty()
    {
        NScopedCriticalSection locker(m_cs);
        return m_queue.empty();
    }

    BOOL try_pop(Data& poppedValue)
    {
        NScopedCriticalSection locker(m_cs);
        if (m_queue.empty())
        {
            return false;
        }
		poppedValue = m_queue.front();
		m_queue.pop();
        return true;
    }

    void waitAndPop(Data& poppedValue)
    {
        NScopedCriticalSection locker(m_cs);
        while (m_queue.empty())
        {
            WaitForSingleObject(m_dataPushed);
        }

        poppedValue = m_queue.front();
        m_queue.pop();
    }
};