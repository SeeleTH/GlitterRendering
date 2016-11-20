#pragma once

#include "../Macro/Macro.h"
#include <string>

#include <iostream>
#include <fstream>

class INStream
{
public:
	enum NSTREAM_ERR
	{
		NSTREAM_ERROR,
		NSTREAM_ENDOFSTREAM,
		NSTREAM_SUCESS
	};

	virtual ~INStream() {}
	virtual UINT64 ReadByte(char* buffer, UINT64 size) = 0;
	virtual UINT64 ReadUInt64() = 0;
	virtual UINT32 ReadUInt32() = 0;
	virtual INT64 ReadInt64() = 0;
	virtual INT32 ReadInt32() = 0;
	virtual float ReadFloat() = 0;
	virtual std::string ReadString() = 0;
	virtual BOOL WriteByte(const char* buffer, const UINT64 size) = 0;
	virtual BOOL WriteUInt64(const UINT64 value) = 0;
	virtual BOOL WriteUInt32(const UINT32 value) = 0;
	virtual BOOL WriteInt64(const INT64 value) = 0;
	virtual BOOL WriteInt32(const INT32 value) = 0;
	virtual BOOL WriteFloat(const float value) = 0;
	virtual BOOL WriteString(const std::string &value) = 0;
	virtual UINT32 CheckErr() = 0;
};

class NFStream : public INStream
{
public:
	enum NFSTREAM_MODE
	{
		NFS_MODE_APP	= 1 << 0,
		NFS_MODE_ATE	= 1 << 1,
		NFS_MODE_BINARY = 1 << 2,
		NFS_MODE_IN		= 1 << 3,
		NFS_MODE_OUT	= 1 << 4,
		NFS_MODE_TRUNC	= 1 << 5,
		NFS_MODE_DEF = NFS_MODE_IN | NFS_MODE_BINARY
	};

	NFStream(std::string path, UINT32 openMode = NFS_MODE_DEF);
	virtual ~NFStream() override;
	virtual UINT64 ReadByte(char* buffer = NULL, UINT64 size = 0) override;
	virtual UINT64 ReadUInt64() override;
	virtual UINT32 ReadUInt32() override;
	virtual INT64 ReadInt64() override;
	virtual INT32 ReadInt32() override;
	virtual float ReadFloat() override;
	virtual std::string ReadString() override;
	virtual BOOL WriteByte(const char* buffer, const UINT64 size) override;
	virtual BOOL WriteUInt64(const UINT64 value) override;
	virtual BOOL WriteUInt32(const UINT32 value) override;
	virtual BOOL WriteInt64(const INT64 value) override;
	virtual BOOL WriteInt32(const INT32 value) override;
	virtual BOOL WriteFloat(const float value) override;
	virtual BOOL WriteString(const std::string &value) override;
	virtual UINT32 CheckErr() override;

protected:
	std::fstream* m_pInOutStream;
};