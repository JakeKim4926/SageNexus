#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"
#include <string>

class HtmlReportExporter
{
public:
    BOOL Export(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);

private:
    BOOL WriteHtmlFile(const CString& strPath, const std::wstring& content, CString& strError) const;

    std::wstring BuildHtml(const DataTable& table, const CString& strLang) const;
    std::wstring BuildHeaderRow(const DataTable& table, const CString& strLang) const;
    std::wstring BuildDataRows(const DataTable& table) const;
    std::wstring EscapeHtml(const CString& str) const;
    CString      GetCurrentTimestamp() const;
};
