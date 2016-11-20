#include "NActorManager.h"


NActorManager::NActorManager()
{
    m_pActorFactory = new NActorFactory();
}

NActorManager::~NActorManager()
{
    FlushActors();
    N_DELETE(m_pActorFactory);
}

void NActorManager::VInit()
{
}

std::weak_ptr<NActor> NActorManager::VGetActor(const NActorId id)
{
    NActorMap::iterator findIt = m_mActors.find(id);
    if (findIt != m_mActors.end())
    {
        std::weak_ptr<NActor> weakPtr(findIt->second);
        return weakPtr;
    }

    return std::weak_ptr<NActor>();
}

std::weak_ptr<NActor> NActorManager::VCreateActor(const char* resource, const SIZE_T resourceSize, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4X4 &rotMatrix)
{
    StrNActorPtr strActor = m_pActorFactory->CreateActor(resource, resourceSize, pos, rotMatrix);
	m_mActors[strActor->GetId()] = strActor;
    std::weak_ptr<NActor> weakActor(strActor);
    return weakActor;
}

void NActorManager::VDestroyActor(const NActorId id)
{
    m_lRemoveActorIds.push_back(id);
}

void NActorManager::VUpdate(float deltaMs)
{
    for (NActorMap::iterator it = m_mActors.begin(); it != m_mActors.end(); it++)
        it->second->Update(deltaMs);
    RemoveActorsInRemoveList();
}

void NActorManager::FlushActors()
{
    //for (NActorMap::iterator it = m_mActors.begin(); it != m_mActors.end(); it++)
    //    it->second->Destroy();
    m_mActors.clear();
}

void NActorManager::RemoveActorsInRemoveList()
{
    for (NActorIdList::iterator it = m_lRemoveActorIds.begin(); it != m_lRemoveActorIds.end(); it++)
    {
        NActorMap::iterator findIt = m_mActors.find(*it);
        if (findIt != m_mActors.end())
        {
            m_mActors.erase(findIt);
        }
    }
}