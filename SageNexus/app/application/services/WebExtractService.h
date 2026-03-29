#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"
#include <string>

class WebExtractService
{
public:
    BOOL FetchAndExtract(const CString& strUrl, const CString& strSelector, DataTable& outTable, CString& strError);

private:
    BOOL FetchHtml(const CString& strUrl, std::string& outHtml, CString& strError);
    BOOL ExtractTable(const std::string& strHtml, const CString& strSelector, DataTable& outTable, CString& strError);

    std::string FindTableHtml(const std::string& strHtml, const CString& strSelector);
    std::vector<std::string> ExtractRowsFromTable(const std::string& strTableHtml);
    std::vector<std::string> ExtractCellsFromRow(const std::string& strRowHtml, const std::string& strCellTag);
    std::string StripHtmlTags(const std::string& str);
    std::string DecodeHtmlEntities(const std::string& str);
    std::string ToLower(const std::string& str);
};
