#pragma once

const uint64_t USERDATA_TYPE_DATATABLE = 0xFFF7FFF700000004;
const uint64_t USERDATA_TYPE_DATATABLE_CUSTOM = 0xFFFCFFFC12345678;

enum class DatatableType : int
{
	BOOL = 0,
	INT,
	FLOAT,
	VECTOR,
	STRING,
	ASSET,
	UNK_STRING // unknown but deffo a string type
};

struct ColumnInfo
{
	char* name;
	DatatableType type;
	int offset;
};

struct Datatable
{
	int numColumns;
	int numRows;
	ColumnInfo* columnInfo;
	char* data; // actually data pointer
	int rowInfo;
};

struct CSVData
{
	std::string m_sAssetName;
	std::string m_sCSVString;
	char* m_pDataBuf;
	size_t m_nDataBufSize;

	std::vector<char*> columns;
	std::vector<std::vector<char*>> dataPointers;
};

void DumpDatatable(const char* pDatatablePath);
