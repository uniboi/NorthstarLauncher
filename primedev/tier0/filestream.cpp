#include "tier0/filestream.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFileStream::CFileStream() : m_nFlags(CFileStream::CLOSED)
{
	// 
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFileStream::~CFileStream()
{
	Close();
}

//-----------------------------------------------------------------------------
// Purpose: Open a new stream
// Input  : &fsPath - Stream path
//          nFlags - Stream flags
// Output : true on success
//-----------------------------------------------------------------------------
bool CFileStream::Open(fs::path& fsPath, int nFlags)
{
	// Make sure we're closed
	Close();

	// Open a new stream
	m_nFlags = nFlags;
	
	m_Stream = std::fstream(fsPath, nFlags);
	if (!m_Stream.is_open() || !m_Stream.good())
	{
		m_nFlags = CFileStream::CLOSED;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Open a new stream
// Input  : *szPath - Stream path
//          nFlags - Stream flags
// Output : true on success
//-----------------------------------------------------------------------------
bool CFileStream::Open(const char* szPath, int nFlags)
{
	if (szPath == nullptr)
		return false;

	fs::path fsPath = szPath;
	Open(fsPath, nFlags);
}

//-----------------------------------------------------------------------------
// Purpose: Close the stream
//-----------------------------------------------------------------------------
void CFileStream::Close()
{
	if (m_Stream.is_open())
	{
		m_Stream.close();
	}

	m_nFlags = CFileStream::CLOSED;
}

//-----------------------------------------------------------------------------
// Purpose: Flush the stream, only works on CFileStream::WRITE
//-----------------------------------------------------------------------------
void CFileStream::Flush()
{
	if (!IsWritable())
		return;

	m_Stream.flush();
}

//-----------------------------------------------------------------------------
// Purpose: Write to the stream
// Input  : tValue - Value to write
//-----------------------------------------------------------------------------
template <typename T>
void CFileStream::Write(T tValue)
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
void CFileStream::Write(T tValue, int nSize)
{
	if (!IsWritable())
		return;

	m_Stream.write(reinterpret_cast<const char*>(&tValue), nSize);
}

//-----------------------------------------------------------------------------
// Purpose: Write string to stream
// Input  : svString - String to write
//-----------------------------------------------------------------------------
void CFileStream::WriteString(std::string svString)
{
	if (!IsWritable())
		return;

	m_Stream.write(svString.c_str(), svString.size());
}

//-----------------------------------------------------------------------------
// Purpose: Read to buffer
// Input  : &tBuffer - Buffer to write to
//-----------------------------------------------------------------------------
template <typename T>
void CFileStream::Read(T& tBuffer)
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
void CFileStream::Read(T& tBuffer, int nSize)
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
T CFileStream::Read()
{
	T tValue {};

	if (!IsReadable())
		return tValue;

	m_Stream.read(reinterpret_cast<char*>(&tValue), sizeof(tValue));

	return tValue;
}

//-----------------------------------------------------------------------------
// Purpose: Read string
// Input  : &svBuffer - String we read into
//-----------------------------------------------------------------------------
void CFileStream::ReadString(std::string& svBuffer)
{
	if (!IsReadable())
		return;

	char c;
	while (!m_Stream.eof() && (c = Read<char>()) != '\0')
		svBuffer += c;
}
