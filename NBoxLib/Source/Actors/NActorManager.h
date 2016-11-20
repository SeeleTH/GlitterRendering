#pragma once

#include "../Macro/Macro.h"
#include "NActor.h"
#include "NActorFactory.h"

typedef std::map<NActorId, StrNActorPtr> NActorMap;
typedef std::list<NActorId> NActorIdList;

class NActorManager
{
public:
    NActorManager();
    ~NActorManager();

    virtual void VInit();
	template<class SubClass>
	void RegisterComponent(const char* name);
    virtual std::weak_ptr<NActor> VGetActor(const NActorId id);
	virtual std::weak_ptr<NActor> VCreateActor(const char* resource, const SIZE_T resourceSize, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4X4 &rotMatrix);
    virtual void VDestroyActor(const NActorId id);
    virtual void VUpdate(float deltaMs);

    void FlushActors();

protected:
    void RemoveActorsInRemoveList();

    NActorMap m_mActors;
    NActorIdList m_lRemoveActorIds;

    NActorFactory* m_pActorFactory;
};

template<class SubClass>
void NActorManager::RegisterComponent(const char* name)
{
	m_pActorFactory->RegisterComponentCreator<SubClass>(name);
}