#pragma once

#include <fstream>

#include "util/filesystem.h"

//-----------------------------------------------------------------------------
// File stream wrapper
//-----------------------------------------------------------------------------
class CFileStream
{
  public:
	enum eMode
	{
		CLOSED = 0,
		READ = std::ios::in,
		WRITE = std::ios::out,
		BINARY = std::ios::binary
	};

	CFileStream();
	~CFileStream();

	bool Open(fs::path& fsPath, int nFlags);
	bool Open(const char* szPath, int nFlags);
	void Close();

	void Flush();

	//-----------------------------------------------------------------------------
	// Writing
	template<typename T>
	void Write(T tValue);

	template <typename T>
	void Write(T tValue, int nSize);

	void WriteString(std::string svString);

	//-----------------------------------------------------------------------------
	// Reading
	template<typename T>
	void Read(T& tBuffer);

	template <typename T>
	void Read(T& tBuffer, int nSize);

	template <typename T>
	T Read();

	void ReadString(std::string& svBuffer);

	//-----------------------------------------------------------------------------
	// Purpose: Check whether the stream is readable
	// Output : true if the stream is readable
	//-----------------------------------------------------------------------------
	bool IsReadable() const
	{
		return (m_nFlags & READ) && m_Stream && !m_Stream.eof();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Check whether the stream is writable
	// Output : true if the stream is writable
	//-----------------------------------------------------------------------------
	bool IsWritable() const
	{
		return (m_nFlags & WRITE) && m_Stream;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Check if we've hit end of file
	// Output : true if we hit eof
	//-----------------------------------------------------------------------------
	bool IsEof() const
	{
		return m_Stream.eof();
	}

  private:
	std::fstream m_Stream;
	int m_nFlags;
};
