#include "NEntity.h"


NEntity::NEntity()
    :m_pParent(NULL)
{

}

NEntity::~NEntity()
{
    if (m_pParent)
        m_pParent->VRemoveEntityChild(this);
}


UINT32 NEntity::VGetEntityType() const
{
    return 0;
}

void NEntity::VSetEntityParent(NEntity * pParent)
{
    m_pParent = pParent;
}

BOOL NEntity::VAddEntityChild(NEntity* pChild)
{
    NEntities::iterator findIt = std::find(m_lEntityChildren.begin(), m_lEntityChildren.end(), pChild);
    if (findIt == m_lEntityChildren.end())
    {
        m_lEntityChildren.push_back(pChild);
        return true;
    }
    return false;
}

BOOL NEntity::VRemoveEntityChild(NEntity* pChild)
{
    NEntities::iterator findIt = std::find(m_lEntityChildren.begin(), m_lEntityChildren.end(), pChild);
    if (findIt == m_lEntityChildren.end())
    {
        m_lEntityChildren.erase(findIt);
        return true;
    }
    return false;
}