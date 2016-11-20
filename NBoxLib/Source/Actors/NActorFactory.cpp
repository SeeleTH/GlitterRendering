#include "NActorFactory.h"

#include "../3rdParty/tinyxml2/tinyxml2.h"

#include "../Debug/NAssert.h"

StrNActorPtr NActorFactory::CreateActor(const char* actorResource, const SIZE_T resourceSize, const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4X4 &rotMatrix)
{
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.Parse(actorResource, resourceSize) != tinyxml2::XML_NO_ERROR)
    {
        N_ERROR("Cannot load actor xml!");
        return StrNActorPtr();
    }

    StrNActorPtr pActor(N_NEW NActor(GetNextActorId()));
    if (!pActor->DataInit(xmlDoc.FirstChildElement("Actor")))
    {
        N_ERROR("Cannot init actor with xml data!");
        return StrNActorPtr();
    }

	pActor->Orientation().Pos() = pos;
	pActor->Orientation().SetRot(rotMatrix);

    for (tinyxml2::XMLElement* pNode = xmlDoc.FirstChildElement("Actor")->FirstChildElement("Components")->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
        StrNActorComponentPtr pComponent(CreateComponent(pNode));
        if (pComponent)
        {
            pActor->AddComponent(pComponent);
            pComponent->SetOwner(pActor);
        }
        else
        {
            return StrNActorPtr();
        }
    }

    pActor->Init();

    return pActor;
}

StrNActorComponentPtr NActorFactory::CreateComponent(NActorElement* pData)
{
    std::string name(pData->Value());

    StrNActorComponentPtr pComponent;

	NActorComponent* created = m_componentFactory.Create(NActorComponent::GetIdFromName(name.c_str()));
	if (created)
    {
		pComponent.reset(created);
    }
    else
    {
        N_ERROR("Couldn't find Actor Component named: " + name);
        return StrNActorComponentPtr();
    }

    if (pComponent)
    {
        if (!pComponent->VOnDataInit(pData))
        {
            N_ERROR("Component failed to initialize: " + name);
            return StrNActorComponentPtr();
        }
    }

    return pComponent;
}