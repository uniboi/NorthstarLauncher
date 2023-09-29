#include "rtech/datatable.h"

#include "mathlib/vector.h"
#include "rtech/pakapi.h"
#include "engine/host.h"

#include "fstream"

std::string DataTableToString(Datatable* datatable)
{
	std::string sCSVString;

	// write columns
	bool bShouldComma = false;
	for (int i = 0; i < datatable->numColumns; i++)
	{
		if (bShouldComma)
			sCSVString += ',';
		else
			bShouldComma = true;

		sCSVString += datatable->columnInfo[i].name;
	}

	// write rows
	for (int row = 0; row < datatable->numRows; row++)
	{
		sCSVString += '\n';

		bool bShouldComma = false;
		for (int col = 0; col < datatable->numColumns; col++)
		{
			if (bShouldComma)
				sCSVString += ',';
			else
				bShouldComma = true;

			// output typed data
			ColumnInfo column = datatable->columnInfo[col];
			const void* pUntypedVal = datatable->data + column.offset + row * datatable->rowInfo;
			switch (column.type)
			{
			case DatatableType::BOOL:
			{
				sCSVString += *(bool*)pUntypedVal ? '1' : '0';
				break;
			}

			case DatatableType::INT:
			{
				sCSVString += std::to_string(*(int*)pUntypedVal);
				break;
			}

			case DatatableType::FLOAT:
			{
				sCSVString += std::to_string(*(float*)pUntypedVal);
				break;
			}

			case DatatableType::VECTOR:
			{
				Vector3* pVector = (Vector3*)(pUntypedVal);
				sCSVString += FormatA("<%f,%f,%f>", pVector->x, pVector->y, pVector->z);
				break;
			}

			case DatatableType::STRING:
			case DatatableType::ASSET:
			case DatatableType::UNK_STRING:
			{
				sCSVString += FormatA("\"%s\"", *(char**)pUntypedVal);
				break;
			}
			}
		}
	}

	return sCSVString;
}

void DumpDatatable(const char* pDatatablePath)
{
	Datatable* pDatatable = (Datatable*)g_pPakLoadManager->LoadFile(pDatatablePath);
	if (!pDatatable)
	{
		Error(eLog::NS, NO_ERROR, "couldn't load datatable {} (rpak containing it may not be loaded?)\n", pDatatablePath);
		return;
	}

	std::string sOutputPath(FormatA("%s/scripts/datatable/%s.csv", g_pEngineParms->szModName, fs::path(pDatatablePath).stem().string().c_str()));
	std::string sDatatableContents(DataTableToString(pDatatable));

	fs::create_directories(fs::path(sOutputPath).remove_filename());
	std::ofstream outputStream(sOutputPath);
	outputStream.write(sDatatableContents.c_str(), sDatatableContents.size());
	outputStream.close();

	DevMsg(eLog::NS, "dumped datatable %s %p to %s\n", pDatatablePath, (void*)pDatatable, sOutputPath.c_str());
}
