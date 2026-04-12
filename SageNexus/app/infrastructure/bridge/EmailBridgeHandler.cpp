#include "pch.h"
#include "app/infrastructure/bridge/EmailBridgeHandler.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"

void EmailBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"workflow.email", L"sendEmail",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleSendEmail(msg);
        });
}

CString EmailBridgeHandler::HandleSendEmail(const BridgeMessage& msg)
{
    EmailAction action;
    action.m_strRecipients     = JsonExtractString(msg.m_strPayloadJson, L"recipients");
    action.m_strSubject        = JsonExtractString(msg.m_strPayloadJson, L"subject");
    action.m_strBody           = JsonExtractString(msg.m_strPayloadJson, L"body");
    action.m_strAttachFilePath = JsonExtractString(msg.m_strPayloadJson, L"attachFilePath");

    if (action.m_strRecipients.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EM_001\",\"message\":\"수신자 주소가 비어 있습니다.\"}}";
    }

    CString strError;
    if (!m_service.SendEmail(action, strError))
    {
        ExecutionHistoryStore histStore;
        ExecutionRecord record;
        record.m_strOperationType = L"email";
        record.m_strSourceName    = action.m_strSubject;
        record.m_strOutputPath    = action.m_strRecipients;
        record.m_bSuccess         = FALSE;
        record.m_strErrorMessage  = strError;
        CString strHistError;
        histStore.SaveRecord(record, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EM_002\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    ExecutionHistoryStore histStore;
    ExecutionRecord record;
    record.m_strOperationType = L"email";
    record.m_strSourceName    = action.m_strSubject;
    record.m_strOutputPath    = action.m_strRecipients;
    record.m_bSuccess         = TRUE;
    CString strHistError;
    histStore.SaveRecord(record, strHistError);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"recipients\":\"" +
           JsonEscapeString(action.m_strRecipients) + L"\"}}";
}
