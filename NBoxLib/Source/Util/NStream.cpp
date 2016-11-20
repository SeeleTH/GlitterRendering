#include "NStream.h"

NFStream::NFStream(std::string path, UINT32 openMode)
{
	m_pInOutStream = new std::fstream();
	UINT32 flag = 0;
	if (openMode & NFS_MODE_APP) flag |= std::fstream::app;
	if (openMode & NFS_MODE_ATE) flag |= std::fstream::ate;
	if (openMode & NFS_MODE_BINARY) flag |= std::fstream::binary;
	if (openMode & NFS_MODE_IN) flag |= std::fstream::in;
	if (openMode & NFS_MODE_OUT) flag |= std::fstream::out;
	if (openMode & NFS_MODE_TRUNC) flag |= std::fstream::trunc;
	m_pInOutStream->open(path, flag);
}

NFStream::~NFStream()
{
	if (m_pInOutStream && m_pInOutStream->is_open())
	{
		m_pInOutStream->close();
	}
	N_DELETE(m_pInOutStream);
}

UINT64 NFStream::ReadByte(char* buffer, UINT64 size)
{
	if (size <= 0 || buffer == NULL)
	{
		std::streampos curPos = m_pInOutStream->tellg();
		m_pInOutStream->seekg(0, std::ios::end);
		std::streampos endPos = m_pInOutStream->tellg();
		m_pInOutStream->seekg(curPos, std::ios::beg);
		m_pInOutStream->clear();
		return endPos - curPos;
	}
	else
	{
		m_pInOutStream->read(buffer, size);
	}

	return CheckErr();
}

UINT64 NFStream::ReadUInt64()
{
	UINT64 value = 0;
	*m_pInOutStream >> value;
	return value;
}

UINT32 NFStream::ReadUInt32()
{
	UINT32 value = 0;
	*m_pInOutStream >> value;
	return value;
}

INT64 NFStream::ReadInt64()
{
	INT64 value = 0;
	*m_pInOutStream >> value;
	return value;
}

INT32 NFStream::ReadInt32()
{
	INT32 value = 0;
	*m_pInOutStream >> value;
	return value;
}

float NFStream::ReadFloat()
{
	float value = 0;
	*m_pInOutStream >> value;
	return value;
}

std::string NFStream::ReadString()
{
	std::string value = "";
	*m_pInOutStream >> value;
	return value;
}

BOOL NFStream::WriteByte(const char* buffer, const UINT64 size)
{
	m_pInOutStream->write(buffer, size);
	return true;
}

BOOL NFStream::WriteUInt64(const UINT64 value)
{
	*m_pInOutStream << value;
	return true;
}

BOOL NFStream::WriteUInt32(const UINT32 value)
{
	*m_pInOutStream << value;
	return true;
}

BOOL NFStream::WriteInt64(const INT64 value)
{
	*m_pInOutStream << value;
	return true;
}

BOOL NFStream::WriteInt32(const INT32 value)
{
	*m_pInOutStream << value;
	return true;
}

BOOL NFStream::WriteFloat(const float value)
{
	*m_pInOutStream << value;
	return true;
}

BOOL NFStream::WriteString(const std::string &value)
{
	*m_pInOutStream << value;
	return true;
}

UINT32 NFStream::CheckErr()
{
	if (m_pInOutStream->good())
		return INStream::NSTREAM_SUCESS;
	if (m_pInOutStream->eof())
		return INStream::NSTREAM_ENDOFSTREAM;
	if (m_pInOutStream->bad())
		return INStream::NSTREAM_ERROR;
	if (m_pInOutStream->fail())
		return INStream::NSTREAM_ERROR;
	return INStream::NSTREAM_SUCESS;
}