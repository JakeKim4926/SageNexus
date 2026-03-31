#include "pch.h"
#include "app/application/services/ExportService.h"
#include "app/infrastructure/writers/CsvWriter.h"
#include "app/infrastructure/writers/XlsxWriter.h"
#include "app/infrastructure/exporters/HtmlReportExporter.h"
#include "app/infrastructure/exporters/WordExporter.h"

BOOL ExportService::ExportToCsv(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
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
    return writer.Write(table, strFilePath, strLang, strError);
}

BOOL ExportService::ExportToXlsx(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
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

    XlsxWriter writer;
    return writer.Write(table, strFilePath, strLang, strError);
}

BOOL ExportService::ExportToHtml(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
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

    HtmlReportExporter exporter;
    return exporter.Export(table, strFilePath, strLang, strError);
}

BOOL ExportService::ExportToWord(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
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

    WordExporter exporter;
    return exporter.Export(table, strFilePath, strLang, strError);
}
