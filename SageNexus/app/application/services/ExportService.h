#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class ExportService
{
public:
    BOOL ExportToCsv(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);
    BOOL ExportToXlsx(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);
    BOOL ExportToHtml(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);
    BOOL ExportToWord(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);
    BOOL ExportToPdf(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);
};
