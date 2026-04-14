#pragma once
#include "pch.h"
#include "app/domain/interfaces/IDataReader.h"

class CsvReader : public IDataReader
{
public:
    CsvReader();
    ~CsvReader();

    BOOL Read(const CString& strFilePath, DataTable& outTable, CString& strError) override;

private:
    std::wstring              ConvertToWide(const std::string& strBytes, UINT nCodePage);
    std::vector<std::wstring> SplitLines(const std::wstring& strContent);
    wchar_t                   DetectDelimiter(const std::wstring& strFirstLine);
    std::vector<CString>      ParseRow(const std::wstring& strLine, wchar_t chDelim);
};
