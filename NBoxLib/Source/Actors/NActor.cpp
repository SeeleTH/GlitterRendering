#include "NActor.h"

#include "../3rdParty/tinyxml2/tinyxml2.h"

NActor::NActor(NActorId id)
    : m_uId(id)
    , m_sName("Unnamed Actor")
    , m_bWorldDirty(true)
    , m_bInheritsTrans(false)
{

}

NActor::~NActor(void)
{
    // ActorManager will be the one, who deal with parent and children of this class.
    Destroy();
}


bool NActor::DataInit(NActorElement* pData)
{
    // @TODO Extra data for actor
    if(strcmp(pData->Value(), "Actor") == 0)
    {
        const NActorElement* cPData = pData;

        const tinyxml2::XMLAttribute* cpAttr = cPData->FindAttribute("Name");

        if (cpAttr)
            m_sName = cpAttr->Value();

        // Get Default Position
        DirectX::XMFLOAT3 actorPos;
        actorPos.x = actorPos.y = actorPos.z = 0.f;
        cpAttr = cPData->FindAttribute("PosX");
        if (cpAttr)
            actorPos.x = cpAttr->FloatValue();
        cpAttr = cPData->FindAttribute("PosY");
        if (cpAttr)
            actorPos.y = cpAttr->FloatValue();
        cpAttr = cPData->FindAttribute("PosZ");
        if (cpAttr)
            actorPos.z = cpAttr->FloatValue();
        Orientation().Pos() = actorPos;

        return true;
    }
    return false;
}

void NActor::Init(void)
{
    VOnInit();
}

void NActor::Destroy(void)
{
    VOnDestroy();

    for (NActorComponents::iterator it = m_mComponents.begin(); it != m_mComponents.end(); it++)
    {
        (*it).second->VOnDestroy();
    }
    m_mComponents.clear();
}

void NActor::Update(float deltaMs)
{
    for (NActorComponents::iterator it = m_mComponents.begin(); it != m_mComponents.end(); it++)
    {
        it->second->VOnUpdate(deltaMs);
    }

    Propagate();
    VOnUpdate(deltaMs);
}

void NActor::Propagate() const
{
    if (m_bWorldDirty)
    {
        for (NEntities::const_iterator it = m_lEntityChildren.begin(); it != m_lEntityChildren.end(); it++)
        {
            NActor* child = (NActor*)(*it);
            if (child && child->m_bInheritsTrans)
            {
                child->SetWorldDirty(true);
                child->Propagate();
            }
        }
    }
}


template <class NComponentType>
std::weak_ptr<NComponentType> NActor::GetComponent(NComponentId id)
{
    NActorComponents::iterator findIt = m_mComponents.find(id);
    if (findIt != m_mComponents.end())
    {
        StrNActorComponentPtr pBase(findIt->second);
        shared_ptr<NComponentType> pSub(std::static_pointer_cast<NComponentType>(pBase));
        std::weak_ptr<ComponentType> pWeakSub(pSub);

        return pWeakSub;
    }
    else
    {
        return std::weak_ptr<ComponentType>();
    }
}

void NActor::AddComponent(StrNActorComponentPtr component)
{
    m_mComponents[component->VGetId()] = component;
    component->VOnInit();
}


void NActor::UndirtyLocal() const
{
    m_oOrientation.UpdateMatrix(m_mLocalTrans);
}

void NActor::UndirtyWorld() const
{
    if (m_bWorldDirty)
    {
        UndirtyLocal();
        if (m_bInheritsTrans && m_pParent)
        {
            NActor* parent = (NActor*)this->GetEntityParent();
            if (parent)
            { 
                DirectX::XMMATRIX parentWorldTrans = DirectX::XMLoadFloat4x4(&parent->WorldTrans());
                DirectX::XMMATRIX localTrans = DirectX::XMLoadFloat4x4(&LocalTrans());
                localTrans = DirectX::XMMatrixMultiply(parentWorldTrans, localTrans);
                DirectX::XMStoreFloat4x4(&m_mWorldTrans, localTrans);
            }
        }
    }

    m_bWorldDirty = false;
}