#include "pch.h"
#include "app/infrastructure/bridge/ApiCallBridgeHandler.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"

void ApiCallBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"workflow.api", L"callApi",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleCallApi(msg);
        });
}

CString ApiCallBridgeHandler::HandleCallApi(const BridgeMessage& msg)
{
    ApiCallAction action;
    action.m_strUrl         = JsonExtractString(msg.m_strPayloadJson, L"url");
    action.m_strMethod      = JsonExtractString(msg.m_strPayloadJson, L"method");
    action.m_strHeadersJson = JsonExtractString(msg.m_strPayloadJson, L"headers");
    action.m_strBody        = JsonExtractString(msg.m_strPayloadJson, L"body");

    CString strTimeout = JsonExtractString(msg.m_strPayloadJson, L"timeout");
    action.m_nTimeoutMs = strTimeout.IsEmpty() ? 30000 : _wtoi(strTimeout);

    if (action.m_strUrl.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_API_001\",\"message\":\"URL이 비어 있습니다.\"}}";
    }

    CString strResponseBody;
    CString strError;
    if (!m_service.CallApi(action, strResponseBody, strError))
    {
        ExecutionHistoryStore histStore;
        ExecutionRecord record;
        record.m_strOperationType = L"api";
        record.m_strSourceName    = action.m_strUrl;
        record.m_strOutputPath    = L"";
        record.m_bSuccess         = FALSE;
        record.m_strErrorMessage  = strError;
        CString strHistError;
        histStore.SaveRecord(record, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_API_002\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    ExecutionHistoryStore histStore;
    ExecutionRecord record;
    record.m_strOperationType = L"api";
    record.m_strSourceName    = action.m_strUrl;
    record.m_strOutputPath    = L"";
    record.m_bSuccess         = TRUE;
    CString strHistError;
    histStore.SaveRecord(record, strHistError);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"response\":\"" +
           JsonEscapeString(strResponseBody) + L"\"}}";
}
