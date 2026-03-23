#include "pch.h"
#include "app/application/services/ExportService.h"
#include "app/infrastructure/writers/CsvWriter.h"

BOOL ExportService::ExportToCsv(const DataTable& table, const CString& strFilePath, CString& strError)
{
    if (table.IsEmpty())
    {
        strError = L"내보낼 데이터가 없습니다.";
        return FALSE;
    }

    if (strFilePath.IsEmpty())
    {
        strError = L"저장 경로가 비어 있습니다.";
        return FALSE;
    }

    CsvWriter writer;
    return writer.Write(table, strFilePath, strError);
}
