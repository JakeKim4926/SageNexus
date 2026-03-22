#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class ImportService
{
public:
    ImportService();
    ~ImportService();

    BOOL LoadFromFile(const CString& strFilePath, DataTable& outTable, CString& strError);
};
