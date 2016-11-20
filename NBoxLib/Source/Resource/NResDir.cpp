#include "NResDir.h"

#include "../Util/NString.h"

#include <algorithm>

NDirResFile::NDirResFile(const std::wstring resPath)
: INResFile()
{
    TCHAR dir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, dir);

    m_sAssetsDir = dir;
	m_sAssetsDir += L"\\";
    //int lastSlash = m_sAssetsDir.find_last_of(L"\\");
    //m_sAssetsDir = m_sAssetsDir.substr(0, lastSlash);
    m_sAssetsDir += resPath;
}


INT32 NDirResFile::Find(const std::string &path)
{
    std::string lowerCase = path;
    std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), (int(*)(int)) tolower);
    DirContentsMap::const_iterator i = m_DirectoryContentsMap.find(lowerCase);
    if (i == m_DirectoryContentsMap.end())
        return -1;

    return i->second;
}

BOOL NDirResFile::VOpen()
{
    ReadAssetsDirectory(L"*");

    return true;
}

INT32 NDirResFile::VGetRawResourceSize(const NRes &r)
{
    INT32 size = 0;

    INT32 num = Find(r.m_sName.c_str());
    if (num < 0)
        return -1;

    return m_AssetFileInfo[num].nFileSizeLow;//+1;
}

INT32 NDirResFile::VGetRawResource(const NRes &r, char *buffer)
{
    INT32 num = Find(r.m_sName.c_str());
    if (num == -1)
        return -1;

    std::string fillFileSpec = ws2s(m_sAssetsDir) + r.m_sName.c_str();
    FILE *f = NULL;
    fopen_s(&f, fillFileSpec.c_str(), "rb");
    size_t bytes = fread(buffer, 1, m_AssetFileInfo[num].nFileSizeLow, f);
    fclose(f);
    //buffer[m_AssetFileInfo[num].nFileSizeLow] = 0;
    return bytes;
}

INT32 NDirResFile::VGetNumResources() const
{
    return m_AssetFileInfo.size();
}

std::string NDirResFile::VGetResourceName(int num) const
{
    return ws2s(m_AssetFileInfo[num].cFileName);
}

void NDirResFile::ReadAssetsDirectory(std::wstring fileSpec)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA findData;

    std::wstring pathSpec = m_sAssetsDir + fileSpec;
    fileHandle = FindFirstFile(pathSpec.c_str(), &findData);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        while (FindNextFile(fileHandle, &findData))
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                continue;

            std::wstring fileName = findData.cFileName;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (fileName != L".." && fileName != L".")
                {
                    fileName = fileSpec.substr(0, fileSpec.length() - 1) + fileName + L"\\*";
                    ReadAssetsDirectory(fileName);
                }
            }
            else
            {
                fileName = fileSpec.substr(0, fileSpec.length() - 1) + fileName;
                std::wstring lower = fileName;
                std::transform(lower.begin(), lower.end(), lower.begin(), (int(*)(int)) tolower);
                wcscpy_s(&findData.cFileName[0], MAX_PATH, lower.c_str());
                m_DirectoryContentsMap[ws2s(lower)] = m_AssetFileInfo.size();
                m_AssetFileInfo.push_back(findData);
            }
        }
    }

    FindClose(fileHandle);
}
