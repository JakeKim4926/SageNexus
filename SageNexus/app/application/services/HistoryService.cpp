#include "pch.h"
#include "app/application/services/HistoryService.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"

HistoryService::HistoryService()
{
}

BOOL HistoryService::GetHistory(std::vector<ExecutionRecord>& arrRecords, CString& strError)
{
    ExecutionHistoryStore store;
    return store.LoadRecords(arrRecords, strError);
}
