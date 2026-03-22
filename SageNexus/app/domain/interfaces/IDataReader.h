#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class IDataReader
{
public:
    virtual ~IDataReader() = default;

    virtual BOOL Read(const CString& strFilePath, DataTable& outTable, CString& strError) = 0;
};
