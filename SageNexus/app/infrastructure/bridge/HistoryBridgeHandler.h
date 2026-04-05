#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/domain/model/ExecutionRecord.h"

class HistoryBridgeHandler
{
public:
    HistoryBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher);

private:
    CString HandleGetHistory(const BridgeMessage& msg);
    CString HandleGetSummary(const BridgeMessage& msg);

    CString SerializeRecords(const std::vector<ExecutionRecord>& arrRecords) const;
    CString SerializeRecord(const ExecutionRecord& record) const;
    CString EscapeJsonString(const CString& str) const;
};
