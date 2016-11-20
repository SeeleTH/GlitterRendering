#pragma once

#include <Windows.h>
#include <assert.h>

#include "../Util/NString.h"

#define N_ASSERT(_EXP_) assert(_EXP_)
#define N_ERROR(_MSG_) \
    {\
    std::string msg(_MSG_); \
    MessageBox(NULL, \
    s2ws(msg).c_str(), \
    L"N ERROR!", \
    MB_ICONWARNING | MB_OK\
    ); \
    }

#define N_DEBUG(__CMD__) __CMD__