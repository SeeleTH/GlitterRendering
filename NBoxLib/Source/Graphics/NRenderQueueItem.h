#pragma once

#include "../Macro/Macro.h"

struct N_RENDER_QUEUE_ITEM
{
    UINT32 m_iShaderId;
    UINT32 m_iTextureIds[N_G_MAX_TEXTURE_UNITS];
};