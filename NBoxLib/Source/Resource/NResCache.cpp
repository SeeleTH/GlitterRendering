#include "NResCache.h"

#include <algorithm>

#include "../Debug/NAssert.h"
#include "../Util/NString.h"

// ============================
// NRes
// ============================

NRes::NRes(const std::string &name)
{
    m_sName = name;
    std::transform(m_sName.begin(), m_sName.end(), m_sName.begin(), (INT32(*)(INT32)) tolower);
}

// ============================
// NResHandle
// ============================

NResHandle::NResHandle(NRes & resource, char *buffer, UINT32 size, NResCache *pResCache)
: m_resource(resource)
{
    m_cBuffer = buffer;
    m_iSize = size;
    m_spExtra = NULL;
    m_pResCache = pResCache;
}

NResHandle::~NResHandle()
{
    N_DELETE_ARRAY(m_cBuffer);
    m_pResCache->MemoryHasBeenFreed(m_iSize);
}

// ============================
// NResCache
// ============================

NResCache::NResCache(const UINT32 sizeInMb, INResFile *resFile)
{
    m_iCacheSize = sizeInMb * 1024 * 1024;
    m_iAllocated = 0;
    m_pFile = resFile;
}

NResCache::~NResCache()
{
    while (!m_lru.empty())
    {
        FreeOneResource();
    }
    N_DELETE(m_pFile);
}

BOOL NResCache::Init()
{
    BOOL retValue = false;
    if (m_pFile->VOpen())
    {
        RegisterLoader(std::shared_ptr<INResLoader>(N_NEW DefaultNResLoader()));
        retValue = true;
    }
    return retValue;
}

void NResCache::RegisterLoader(std::shared_ptr<INResLoader> loader)
{
    m_resourceLoaders.push_front(loader);
}

std::shared_ptr<NResHandle> NResCache::GetHandle(NRes * r)
{
    std::shared_ptr<NResHandle> handle(Find(r));
    if (handle == NULL)
    {
        handle = Load(r);
        N_ASSERT(handle);
    }
    else
    {
        Update(handle);
    }
    return handle;
}

std::shared_ptr<NResHandle> NResCache::Load(NRes * r)
{
    std::shared_ptr<INResLoader> loader;
    std::shared_ptr<NResHandle> handle;

    for (NResLoaders::iterator it = m_resourceLoaders.begin(); it != m_resourceLoaders.end(); it++)
    {
        std::shared_ptr<INResLoader> testLoader = *it;

        if (WildcardMatch(testLoader->VGetPattern().c_str(), r->m_sName.c_str()))
        {
            loader = testLoader;
            break;
        }
    }

    if (!loader)
    {
        N_ASSERT(loader && _T("Default resource loader not found!"));
        return handle;
    }

    INT32 rawSize = m_pFile->VGetRawResourceSize(*r);
    if (rawSize < 0)
    {
        N_ASSERT(rawSize > 0 && "Resource size returned -1 - Resource not found");
        return std::shared_ptr<NResHandle>();
    }

    INT32 allocSize = rawSize + ((loader->VAddNullZero()) ? (1) : (0));
    char *rawBuffer = loader->VUseRawFile() ? Allocate(allocSize) : N_NEW char[allocSize];
    memset(rawBuffer, 0, allocSize);

    if (rawBuffer == NULL || m_pFile->VGetRawResource(*r, rawBuffer) == 0)
    {
        return std::shared_ptr<NResHandle>();
    }

    char *buffer = NULL;
    UINT32 size = 0;

    if (loader->VUseRawFile())
    {
        buffer = rawBuffer;
        handle = std::shared_ptr<NResHandle>(N_NEW NResHandle(*r, buffer, rawSize, this));
    }
    else
    {
        size = loader->VGetLoadedResourceSize(rawBuffer, rawSize);
        buffer = Allocate(size);
        if (rawBuffer == NULL || buffer == NULL)
        {
            return std::shared_ptr<NResHandle>();
        }
        handle = std::shared_ptr<NResHandle>(N_NEW NResHandle(*r, buffer, size, this));
        BOOL success = loader->VLoadResource(rawBuffer, rawSize, handle);

        if (loader->VDiscardRawBufferAfterLoad())
        {
            N_DELETE_ARRAY(rawBuffer);
        }

        if (!success)
        {
            return std::shared_ptr<NResHandle>();
        }
    }
    
    if (handle)
    {
        m_lru.push_front(handle);
        m_resources[r->m_sName] = handle;
    }

    N_ASSERT(loader && _T("Default resource loader not found!"));
    return handle;
}

std::shared_ptr<NResHandle> NResCache::Find(NRes * r)
{
    NResHandleMap::iterator i = m_resources.find(r->m_sName);
    if (i == m_resources.end())
        return std::shared_ptr<NResHandle>();

    return i->second;
}

void NResCache::Update(std::shared_ptr<NResHandle> handle)
{
    m_lru.remove(handle);
    m_lru.push_front(handle);
}

char *NResCache::Allocate(UINT32 size)
{
    if (!MakeRoom(size))
        return NULL;

    char *mem = N_NEW char[size];
    if (mem)
    {
        m_iAllocated += size;
    }

    return mem;
}

void NResCache::FreeOneResource()
{
    NResHandles::iterator gonner = m_lru.end();
    gonner--;

    std::shared_ptr<NResHandle> handle = *gonner;

    m_lru.pop_back();
    m_resources.erase(handle->m_resource.m_sName);
}

void NResCache::Flush()
{
    while (!m_lru.empty())
    {
        std::shared_ptr<NResHandle> handle = *(m_lru.begin());
        Free(handle);
    }
}

BOOL NResCache::MakeRoom(UINT32 size)
{
    if (size > m_iCacheSize)
    {
        return false;
    }

    while (size > (m_iCacheSize - m_iAllocated))
    {
        if (m_lru.empty())
            return false;

        FreeOneResource();
    }

    return true;
}

void NResCache::Free(std::shared_ptr<NResHandle> gonner)
{
    m_lru.remove(gonner);
    m_resources.erase(gonner->m_resource.m_sName);
}

void NResCache::MemoryHasBeenFreed(UINT32 size)
{
    m_iAllocated -= size;
}

std::vector<std::string> NResCache::Match(const std::string pattern)
{
    std::vector<std::string> matchingNames;
    if (m_pFile == NULL)
        return matchingNames;

    int numFiles = m_pFile->VGetNumResources();
    for (int i = 0; i < numFiles; i++)
    {
        std::string name = m_pFile->VGetResourceName(i);
        std::transform(name.begin(), name.end(), name.begin(), (int(*)(int)) tolower);
        if (WildcardMatch(pattern.c_str(), name.c_str()))
        {
            matchingNames.push_back(name);
        }
    }
    return matchingNames;
}

INT32 NResCache::Preload(const std::string pattern, void(*progressCallback)(INT32, BOOL &))
{
    if (m_pFile == NULL)
        return 0;

    INT32 numFile = m_pFile->VGetNumResources();
    INT32 loaded = 0;
    BOOL cancel = false;
    for (int i = 0; i < numFile; i++)
    {
        NRes resource(m_pFile->VGetResourceName(i));

        if (WildcardMatch(pattern.c_str(), resource.m_sName.c_str()))
        {
            std::shared_ptr<NResHandle> handle = GetHandle(&resource);
            ++loaded;
        }

        if (progressCallback != NULL)
        {
            progressCallback(i * 100 / numFile, cancel);
        }
    }
    return loaded;
}