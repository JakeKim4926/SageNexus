#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class ExportService
{
public:
    BOOL ExportToCsv(const DataTable& table, const CString& strFilePath, CString& strError);
};
