#pragma once

#include "../Macro/Macro.h"
#include "../Debug/NAssert.h"
#include <cstdlib>

template<typename T>
class NSingleton {
public:
    static T& Instance();
    static void Destroy();

protected:
    inline explicit NSingleton() {
        N_ASSERT(NSingleton::instance_ == 0);
        NSingleton::instance_ = static_cast<T*>(this);
    }
    inline ~NSingleton() {
        NSingleton::instance_ = 0;
    }

private:
    static T* CreateInstance();
    static void ScheduleForDestruction(void(*)());
    static void DestroyInstance(T*);

private:
    static T* instance_;

private:
    inline explicit NSingleton(NSingleton const&) {}
    inline NSingleton& operator=(NSingleton const&) { return *this; }
};    

// end of class NSESingleton

template<typename T>
typename T& NSingleton<T>::Instance() {
    if (NSingleton::instance_ == 0) {
        NSingleton::instance_ = CreateInstance();
        ScheduleForDestruction(NSingleton::Destroy);
    }
    return *(NSingleton::instance_);
}

template<typename T>
void NSingleton<T>::Destroy() {
    if (NSingleton::instance_ != 0) {
        DestroyInstance(NSingleton::instance_);
        NSingleton::instance_ = 0;
    }
}

template<typename T>
inline typename T* NSingleton<T>::CreateInstance() {
    return new T();
}

template<typename T>
inline void NSingleton<T>::ScheduleForDestruction(void(*pFun)()) {
    std::atexit(pFun);
}

template<typename T>
inline void NSingleton<T>::DestroyInstance(T* p) {
    delete p;
}

template<typename T>
typename T* NSingleton<T>::instance_ = 0;