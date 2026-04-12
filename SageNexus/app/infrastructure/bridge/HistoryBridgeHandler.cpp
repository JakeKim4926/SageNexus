#include "pch.h"
#include "app/infrastructure/bridge/HistoryBridgeHandler.h"
#include "app/application/services/HistoryService.h"

HistoryBridgeHandler::HistoryBridgeHandler()
{
}

void HistoryBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"history.query", L"getHistory",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetHistory(msg);
        });

    dispatcher.RegisterHandler(L"execution.summary", L"getSummary",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetSummary(msg);
        });
}

CString HistoryBridgeHandler::HandleGetHistory(const BridgeMessage& msg)
{
    HistoryService service;
    std::vector<ExecutionRecord> arrRecords;
    CString strError;

    if (!service.GetHistory(arrRecords, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_HI_001\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    CString strRecordsJson = SerializeRecords(arrRecords);
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"records\":" + strRecordsJson + L"}}";
}

CString HistoryBridgeHandler::HandleGetSummary(const BridgeMessage& msg)
{
    HistoryService service;
    std::vector<ExecutionRecord> arrRecords;
    CString strError;

    if (!service.GetHistory(arrRecords, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"SNX_HI_002\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    int nTotal   = (int)arrRecords.size();
    int nSuccess = 0;
    int nFailed  = 0;

    for (const ExecutionRecord& rec : arrRecords)
    {
        if (rec.m_bSuccess) ++nSuccess;
        else                ++nFailed;
    }

    CString strPayload;
    strPayload.Format(L"{\"total\":%d,\"success\":%d,\"failed\":%d}", nTotal, nSuccess, nFailed);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strPayload + L"}";
}

CString HistoryBridgeHandler::SerializeRecords(const std::vector<ExecutionRecord>& arrRecords) const
{
    CString strJson = L"[";
    for (int i = 0; i < (int)arrRecords.size(); ++i)
    {
        if (i > 0) strJson += L",";
        strJson += SerializeRecord(arrRecords[i]);
    }
    strJson += L"]";
    return strJson;
}

CString HistoryBridgeHandler::SerializeRecord(const ExecutionRecord& record) const
{
    wchar_t buf[32];
    CString strJson = L"{";
    strJson += L"\"runId\":\"" + JsonEscapeString(record.m_strRunId) + L"\",";
    strJson += L"\"timestamp\":\"" + JsonEscapeString(record.m_strTimestamp) + L"\",";
    strJson += L"\"operationType\":\"" + JsonEscapeString(record.m_strOperationType) + L"\",";
    strJson += L"\"sourceName\":\"" + JsonEscapeString(record.m_strSourceName) + L"\",";
    swprintf_s(buf, 32, L"%d", record.m_nRowCount);
    strJson += L"\"rowCount\":"; strJson += buf; strJson += L",";
    swprintf_s(buf, 32, L"%d", record.m_nColumnCount);
    strJson += L"\"columnCount\":"; strJson += buf; strJson += L",";
    strJson += L"\"outputPath\":\"" + JsonEscapeString(record.m_strOutputPath) + L"\",";
    strJson += L"\"success\":";
    strJson += (record.m_bSuccess ? L"true" : L"false");
    strJson += L"}";
    return strJson;
}
