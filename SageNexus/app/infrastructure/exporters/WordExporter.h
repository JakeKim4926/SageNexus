#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"
#include <string>

class WordExporter
{
public:
    BOOL Export(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);

private:
    BOOL WriteDocxFiles(const CString& strSrcDir, const DataTable& table, const CString& strLang, CString& strError) const;
    BOOL WriteTextFile(const CString& strPath, const std::wstring& content, CString& strError) const;
    BOOL CompressToDocx(const CString& strSrcDir, const CString& strDestPath, CString& strError) const;
    void CleanupDir(const CString& strDir) const;

    std::wstring BuildContentTypes() const;
    std::wstring BuildRootRels() const;
    std::wstring BuildDocumentRels() const;
    std::wstring BuildDocument(const DataTable& table, const CString& strLang) const;
    std::wstring BuildStyles() const;

    std::wstring BuildHeaderRow(const DataTable& table, const CString& strLang) const;
    std::wstring BuildDataRows(const DataTable& table) const;
    std::wstring EscapeXml(const CString& str) const;
    CString      GetCurrentTimestamp() const;
};
