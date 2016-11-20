#include "NAssetStreamer.h"

#include <Windows.h>
#include <locale>
#include <codecvt>

INStream* NFileAssetStreamer::GetStream(std::string key)
{
	TCHAR dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, dir);
	std::string curDir = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(dir);
	NFStream* stream = new NFStream(curDir + "\\" + key);
	return stream;
}