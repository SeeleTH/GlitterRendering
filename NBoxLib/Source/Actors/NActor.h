#pragma once

#include "../Macro/Macro.h"
#include "NEntity.h"
#include "../Util/NOrientation.h"
#include "../Util/NString.h"

#include <memory>
#include <map>
#include <list>

#include <DirectXMath.h>

// ===========================
// Forward Declaration - Begin
// ===========================
class NActor;
class NActorComponent;
class NActorFactory;

namespace tinyxml2
{
    class XMLElement;
}
// ===========================
// Forward Declaration - End
// ===========================

// ===========================
// Typedef - Begin
// ===========================
typedef tinyxml2::XMLElement NActorElement;
typedef unsigned long NActorId;
typedef unsigned long NComponentId;
typedef std::shared_ptr<NActor> StrNActorPtr;
typedef std::shared_ptr<NActorComponent> StrNActorComponentPtr;
// ===========================
// Typedef - End
// ===========================

class NActor : public NEntity
{
protected:
    friend class NActorFactory;
    typedef std::map<NComponentId, StrNActorComponentPtr> NActorComponents;

public:
    explicit NActor(NActorId id);
    ~NActor(void);

    bool DataInit(NActorElement* pData);
    void Init(void);
    void Destroy(void);
    void Update(float deltaMs);
    
    virtual void VOnInit() {}
    virtual void VOnUpdate(float deltaMs) {}
    virtual void VOnDestroy() {}

    NActorId GetId(void) const      { return m_uId;     }
    std::string GetName(void) const { return m_sName;   }

    inline const DirectX::XMFLOAT4X4 & LocalTrans() const;
    inline const DirectX::XMFLOAT4X4 & WorldTrans() const;
    inline const NOrientation & Orientation() const { return m_oOrientation;                        }
    inline NOrientation & Orientation()             { m_bWorldDirty = true; return m_oOrientation;  }
    inline void SetWorldDirty(bool dirty = true) const  { m_bWorldDirty = dirty;    }
    inline BOOL GetWorldDirty() const                   { return m_bWorldDirty;     }
    void Propagate() const;

    template <class NComponentType>
    std::weak_ptr<NComponentType> GetComponent(NComponentId id);
    void AddComponent(StrNActorComponentPtr component);

protected:
    virtual void UndirtyLocal() const;
    virtual void UndirtyWorld() const;

    NActorId            m_uId;
    NActorComponents    m_mComponents;

    std::string         m_sName;

    NOrientation                m_oOrientation;
    mutable DirectX::XMFLOAT4X4 m_mLocalTrans;
    mutable DirectX::XMFLOAT4X4 m_mWorldTrans;
    mutable BOOL                m_bWorldDirty;
    BOOL                        m_bInheritsTrans;
};

const DirectX::XMFLOAT4X4 & NActor::LocalTrans() const
{
    UndirtyLocal();
    return m_mLocalTrans;
}

const DirectX::XMFLOAT4X4 & NActor::WorldTrans() const
{
    if (m_bInheritsTrans)
    {
        UndirtyWorld();
        return m_mWorldTrans;
    }
    return LocalTrans();
}

class NActorComponent
{
    friend class NActorFactory;

public:
    virtual ~NActorComponent(void) { }

    virtual bool VOnDataInit(NActorElement* pData) = 0;
    virtual void VOnInit(void) { }
    virtual void VOnUpdate(float deltaMs) { }
    virtual void VOnDestroy(void) { }

	inline virtual const char* VGetName() const = 0;
	inline virtual NComponentId VGetId(void) const { return GetIdFromName(VGetName()); }

	inline static NComponentId GetIdFromName(const char* componentStr)
	{
		void* rawId = HashedString::hash_name(componentStr);
		return reinterpret_cast<NComponentId>(rawId);
	}

protected:
    StrNActorPtr m_pOwner;

private:
    void SetOwner(StrNActorPtr pOwner) { m_pOwner = pOwner; }
};