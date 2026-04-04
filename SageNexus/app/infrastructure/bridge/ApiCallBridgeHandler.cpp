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
    action.m_strUrl         = ExtractPayloadString(msg.m_strPayloadJson, L"url");
    action.m_strMethod      = ExtractPayloadString(msg.m_strPayloadJson, L"method");
    action.m_strHeadersJson = ExtractPayloadString(msg.m_strPayloadJson, L"headers");
    action.m_strBody        = ExtractPayloadString(msg.m_strPayloadJson, L"body");

    CString strTimeout = ExtractPayloadString(msg.m_strPayloadJson, L"timeout");
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
               EscapeJson(strError) + L"\"}}";
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
           EscapeJson(strResponseBody) + L"\"}}";
}

CString ApiCallBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
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

CString ApiCallBridgeHandler::EscapeJson(const CString& str) const
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
