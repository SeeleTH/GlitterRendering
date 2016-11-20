#pragma once

#include "../Macro/Macro.h"

class NRenderableObject
{
public:
    virtual UINTPTR VGetRenderData() = 0;
    virtual UINT32 VGetSortKey() = 0;
};