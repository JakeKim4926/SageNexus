#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class CsvWriter
{
public:
    BOOL Write(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);

private:
    CString QuoteField(const CString& str) const;
    BOOL NeedsQuoting(const CString& str) const;
};
