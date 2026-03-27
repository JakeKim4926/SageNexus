#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"
#include <string>
#include <vector>

class XlsxWriter
{
public:
    BOOL Write(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);

private:
    BOOL WriteXmlFiles(const CString& strSrcDir, const DataTable& table, const CString& strLang, CString& strError) const;
    BOOL WriteTextFile(const CString& strPath, const std::wstring& content, CString& strError) const;
    BOOL CompressToXlsx(const CString& strSrcDir, const CString& strDestPath, CString& strError) const;
    void CleanupDir(const CString& strDir) const;

    std::wstring BuildContentTypes() const;
    std::wstring BuildRootRels() const;
    std::wstring BuildWorkbook() const;
    std::wstring BuildWorkbookRels() const;
    std::wstring BuildStyles() const;
    std::wstring BuildSheet(const DataTable& table, const CString& strLang) const;

    CString      ColIndexToRef(int nColIdx) const;
    std::wstring EscapeXml(const CString& str) const;
};
