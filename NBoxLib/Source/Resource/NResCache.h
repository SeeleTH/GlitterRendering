#pragma once
#include "../Macro/Macro.h"

#include <string>
#include <memory>
#include <list>
#include <map>
#include <vector>

class NResHandle;

// Interface for NResLoader class
// This class will post process loaded file buffer
class INResLoader
{
public:
	virtual std::string VGetPattern() = 0;
    virtual BOOL VUseRawFile() = 0;
    virtual BOOL VDiscardRawBufferAfterLoad() = 0;
    virtual BOOL VAddNullZero() { return false; }
    virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) = 0;
    virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle) = 0;
};

// Default loader
class DefaultNResLoader : public INResLoader
{
public:
    virtual BOOL VUseRawFile() { return true; }
    virtual BOOL VDiscardRawBufferAfterLoad() { return false; }
    virtual UINT32 VGetLoadedResourceSize(char *rawBuffer, UINT32 rawSize) { return rawSize; }
    virtual BOOL VLoadResource(char *rawBuffer, UINT32 rawSize, std::shared_ptr<NResHandle> handle) { return true; }
    virtual std::string VGetPattern() { return "*"; }
};

class NRes;

// Interface for NResFile class
// This class will load file content into file buffer
class INResFile
{
public:
    virtual BOOL VOpen() = 0;
    virtual INT32 VGetRawResourceSize(const NRes &r) = 0;
    virtual INT32 VGetRawResource(const NRes &r, char *buffer) = 0;
    virtual INT32 VGetNumResources() const = 0;
    virtual std::string VGetResourceName(INT32 num) const = 0;
    virtual BOOL VIsUsingDevelopmentDirectories(void) const = 0;
    virtual ~INResFile() { }
};

// Resource's extra data
class INResExtraData
{
public:
	virtual std::string VToString() = 0;
};

// Resource identifier
class NRes
{
public:
    std::string m_sName;
    NRes(const std::string &name);
};

class NResCache;

// Resource Handle
class NResHandle
{
    friend class NResCache;

protected:
    NRes m_resource;
    char* m_cBuffer;
    UINT32 m_iSize;
    std::shared_ptr<INResExtraData> m_spExtra;
    NResCache* m_pResCache;

public:
    NResHandle(NRes & resource, char *buffer, UINT32 size, NResCache *pResCache);

    virtual ~NResHandle();

    const std::string GetName() { return m_resource.m_sName; }
    UINT32 GetSize() const { return m_iSize; }
    char *GetBuffer() const { return m_cBuffer; }
    char *GetWritableBuffer() { return m_cBuffer; }

    std::shared_ptr<INResExtraData> GetExtra() { return m_spExtra; }
    void SetExtra(std::shared_ptr<INResExtraData> extra) { m_spExtra = extra; }
};

// Resource Cache Manager

typedef std::list< std::shared_ptr< NResHandle > > NResHandles;
typedef std::map < std::string, std::shared_ptr< NResHandle > > NResHandleMap;
typedef std::list< std::shared_ptr< INResLoader > > NResLoaders;

class NResCache
{
    friend class NResHandle;

private:
    NResHandles m_lru;
    NResHandleMap m_resources;
    NResLoaders m_resourceLoaders;

    INResFile *m_pFile;

    UINT32 m_iCacheSize;
    UINT32 m_iAllocated;

protected:
    BOOL MakeRoom(UINT32 size);
    char *Allocate(UINT32 size);
    void Free(std::shared_ptr<NResHandle> gonner);
    
    std::shared_ptr<NResHandle> Load(NRes * r);
    std::shared_ptr<NResHandle> Find(NRes * r);
    void Update(std::shared_ptr<NResHandle> handle);

    void FreeOneResource();
    void MemoryHasBeenFreed(UINT32 size);

public:
    NResCache(const UINT32 sizeInMb, INResFile *file);
    virtual ~NResCache();

    BOOL Init();

    void RegisterLoader(std::shared_ptr<INResLoader> loader);

    std::shared_ptr<NResHandle> GetHandle(NRes * r);

    INT32 Preload(const std::string pattern, void(*progressCallback)(INT32, BOOL &));
    std::vector<std::string> Match(const std::string pattern);

    void Flush(void);

    BOOL IsUsingDevelopmentDirectories(void) const;
};