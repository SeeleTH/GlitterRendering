#pragma once

#include "../Macro/Macro.h"

#include <list>

class NEntity
{
public:
    NEntity();
    virtual ~NEntity();

    virtual UINT32 VGetEntityType() const;
    virtual void VSetEntityParent(NEntity * pParent);
    virtual BOOL VAddEntityChild(NEntity* pChild);
    virtual BOOL VRemoveEntityChild(NEntity* pChild);

    inline NEntity* GetEntityParent();
    inline const NEntity* GetEntityParent() const;
    inline NEntity* GetEntityAncestor() const;

protected:
    typedef std::list<NEntity *> NEntities;
    NEntities m_lEntityChildren;
    NEntity* m_pParent;
};

inline NEntity* NEntity::GetEntityParent()
{
    return m_pParent;
}

inline const NEntity* NEntity::GetEntityParent() const
{
    return m_pParent;
}

inline NEntity* NEntity::GetEntityAncestor() const
{
    NEntity* pAncestor = m_pParent;
    while (pAncestor)
        pAncestor = pAncestor->m_pParent;
    return pAncestor;
}
