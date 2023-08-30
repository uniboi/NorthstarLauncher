#pragma once

#include <fstream>

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
	// Purpose: Write to the stream
	// Input  : tValue - Value to write
	//-----------------------------------------------------------------------------
	template <typename T>
	void Write(T tValue)
	{
		if (!IsWritable())
			return;

		m_Stream.write(reinterpret_cast<const char*>(&tValue), sizeof(tValue));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Write to the stream
	// Input  : tValue - Value to write
	//          nSize - Size of tValue
	//-----------------------------------------------------------------------------
	template <typename T>
	void Write(T tValue, int nSize)
	{
		if (!IsWritable())
			return;

		m_Stream.write(reinterpret_cast<const char*>(&tValue), nSize);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Write to the stream
	// Input  : *tValue - Value to write
	//          nSize - Size of tValue
	//-----------------------------------------------------------------------------
	template <typename T>
	void Write(T* tValue, int nSize)
	{
		if (!IsWritable())
			return;

		m_Stream.write(reinterpret_cast<const char*>(tValue), nSize);
	}

	void WriteString(std::string svString);

	//-----------------------------------------------------------------------------
	// Purpose: Read to buffer
	// Input  : &tBuffer - Buffer to write to
	//-----------------------------------------------------------------------------
	template <typename T>
	void Read(T& tBuffer)
	{
		if (!IsReadable())
			return;

		m_Stream.read(reinterpret_cast<char*>(&tBuffer), sizeof(tBuffer));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Read to buffer
	// Input  : &tBuffer - Buffer to write to
	//          nSize - Size of buffer
	//-----------------------------------------------------------------------------
	template <typename T>
	void Read(T& tBuffer, int nSize)
	{
		if (!IsReadable())
			return;

		m_Stream.read(reinterpret_cast<char*>(&tBuffer), nSize);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Read value
	// Output : Value we read
	//-----------------------------------------------------------------------------
	template <typename T>
	T Read()
	{
		T tValue {};

		if (!IsReadable())
			return tValue;

		m_Stream.read(reinterpret_cast<char*>(&tValue), sizeof(tValue));

		return tValue;
	}

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
