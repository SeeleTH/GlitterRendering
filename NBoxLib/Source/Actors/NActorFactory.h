#pragma once

#include "../Macro/Macro.h"
#include "NActor.h"
#include "../Template/NGenericObjectFactory.h"

class NActorFactory
{
public:
    NActorFactory(void){}
	StrNActorPtr CreateActor(const char* actorResource, const SIZE_T resourceSize, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4X4 &rotMatrix);
    
	template <class SubClass>
	void RegisterComponentCreator(const char* name);

protected:
    virtual StrNActorComponentPtr CreateComponent(NActorElement* pData);

    NActorId m_uLastActorId;

	NGenericObjectFactory<NActorComponent, NComponentId> m_componentFactory;

private:
    NActorId GetNextActorId(void) { return ++m_uLastActorId; }

};



template <class SubClass>
void NActorFactory::RegisterComponentCreator(const char* name)
{
	m_componentFactory.Register<SubClass>(NActorComponent::GetIdFromName(name));
}