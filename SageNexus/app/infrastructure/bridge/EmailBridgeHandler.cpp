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
    action.m_strRecipients     = ExtractPayloadString(msg.m_strPayloadJson, L"recipients");
    action.m_strSubject        = ExtractPayloadString(msg.m_strPayloadJson, L"subject");
    action.m_strBody           = ExtractPayloadString(msg.m_strPayloadJson, L"body");
    action.m_strAttachFilePath = ExtractPayloadString(msg.m_strPayloadJson, L"attachFilePath");

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
               EscapeJson(strError) + L"\"}}";
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
           EscapeJson(action.m_strRecipients) + L"\"}}";
}

CString EmailBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return L"";

    size_t nQuoteOpen = json.find('"', nColon + 1);
    if (nQuoteOpen == std::string::npos) return L"";

    size_t nQuoteClose = nQuoteOpen + 1;
    while (nQuoteClose < json.size())
    {
        if (json[nQuoteClose] == '\\') { nQuoteClose += 2; continue; }
        if (json[nQuoteClose] == '"') break;
        ++nQuoteClose;
    }

    if (nQuoteClose >= json.size()) return L"";

    std::string val = json.substr(nQuoteOpen + 1, nQuoteClose - nQuoteOpen - 1);

    std::string unescaped;
    for (size_t i = 0; i < val.size(); ++i)
    {
        if (val[i] == '\\' && i + 1 < val.size())
        {
            ++i;
            switch (val[i])
            {
            case '"':  unescaped += '"';  break;
            case '\\': unescaped += '\\'; break;
            case '/':  unescaped += '/';  break;
            case 'n':  unescaped += '\n'; break;
            case 'r':  unescaped += '\r'; break;
            case 't':  unescaped += '\t'; break;
            default:   unescaped += val[i]; break;
            }
        }
        else
        {
            unescaped += val[i];
        }
    }

    return Utf8ToWide(unescaped);
}

CString EmailBridgeHandler::EscapeJson(const CString& str) const
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        switch (ch)
        {
        case L'"':  strResult += L"\\\""; break;
        case L'\\': strResult += L"\\\\"; break;
        case L'\n': strResult += L"\\n";  break;
        case L'\r': strResult += L"\\r";  break;
        case L'\t': strResult += L"\\t";  break;
        default:    strResult += ch;      break;
        }
    }
    return strResult;
}
