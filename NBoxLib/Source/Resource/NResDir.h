#pragma once
#include "NResCache.h"
#include <Windows.h>

class NDirResFile : public INResFile
{
protected:
	typedef std::map<std::string, int> DirContentsMap;

public:
    std::wstring m_sAssetsDir;
    std::vector<WIN32_FIND_DATA> m_AssetFileInfo;
    DirContentsMap m_DirectoryContentsMap;

    NDirResFile(const std::wstring resPath);

    virtual BOOL VOpen();
    virtual INT32 VGetRawResourceSize(const NRes &r);
    virtual INT32 VGetRawResource(const NRes &r, char *buffer);
    virtual INT32 VGetNumResources() const;
	virtual std::string VGetResourceName(int num) const;
    virtual BOOL VIsUsingDevelopmentDirectories(void) const { return true; }

    INT32 Find(const std::string &path);

protected:
    void ReadAssetsDirectory(std::wstring fileSpec);
};