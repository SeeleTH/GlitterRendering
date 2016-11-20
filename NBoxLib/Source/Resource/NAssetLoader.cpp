#include "NAssetLoader.h"
#include "../Util/NStream.h"
#include <sys/stat.h>


NFileAssetLoader::NFileAssetLoader(std::string assetDir)
	:m_sAssetDir(assetDir)
{

}

NFileAssetLoader::~NFileAssetLoader()
{

}

BOOL NFileAssetLoader::GetAssetData(std::string id, char* buffer, UINT32 size)
{
	if (size <= 0)
		return false;
	std::string filePath = m_sAssetDir + id;
	NFStream stream(filePath);
	stream.ReadByte(buffer, size);
	return stream.CheckErr() == INStream::NSTREAM_SUCESS;
}

UINT64 NFileAssetLoader::GetAssetSize(std::string id)
{
	std::string filePath = m_sAssetDir + id;
	return GetFileSize(id);
}

UINT64 NFileAssetLoader::GetFileSize(std::string path)
{
	struct stat stat_buf;
	INT32 rc = stat(path.c_str(), &stat_buf);
	if (!rc)
		return -1;
	return stat_buf.st_size;
}
