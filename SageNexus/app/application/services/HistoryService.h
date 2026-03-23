#pragma once
#include "pch.h"
#include "app/domain/model/ExecutionRecord.h"

class HistoryService
{
public:
    HistoryService();

    BOOL GetHistory(std::vector<ExecutionRecord>& arrRecords, CString& strError);
};
