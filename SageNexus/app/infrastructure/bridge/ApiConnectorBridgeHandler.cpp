#include "pch.h"
#include "app/infrastructure/bridge/ApiConnectorBridgeHandler.h"
#include "app/application/SageApp.h"
#include <string>
#include <vector>

ApiConnectorBridgeHandler::ApiConnectorBridgeHandler()
{
    m_service.LoadFromFile();
}

void ApiConnectorBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"connector", L"getConnectors",
        [this](const BridgeMessage& msg) -> CString { return HandleGetConnectors(msg); });

    dispatcher.RegisterHandler(L"connector", L"addConnector",
        [this](const BridgeMessage& msg) -> CString { return HandleAddConnector(msg); });

    dispatcher.RegisterHandler(L"connector", L"removeConnector",
        [this](const BridgeMessage& msg) -> CString { return HandleRemoveConnector(msg); });

    dispatcher.RegisterHandler(L"connector", L"updateConnector",
        [this](const BridgeMessage& msg) -> CString { return HandleUpdateConnector(msg); });

    dispatcher.RegisterHandler(L"connector", L"testConnector",
        [this](const BridgeMessage& msg) -> CString { return HandleTestConnector(msg); });
}

CString ApiConnectorBridgeHandler::HandleGetConnectors(const BridgeMessage& msg)
{
    std::vector<ApiConnector> arrConnectors;
    m_service.GetConnectors(arrConnectors);

    CString strArray = L"[";
    for (int i = 0; i < (int)arrConnectors.size(); ++i)
    {
        if (i > 0) strArray += L",";
        strArray += SerializeConnector(arrConnectors[i]);
    }
    strArray += L"]";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"connectors\":" + strArray + L"}}";
}

CString ApiConnectorBridgeHandler::HandleAddConnector(const BridgeMessage& msg)
{
    CString strName        = ExtractPayloadString(msg.m_strPayloadJson, L"name");
    CString strBaseUrl     = ExtractPayloadString(msg.m_strPayloadJson, L"baseUrl");
    CString strHeadersJson = ExtractPayloadString(msg.m_strPayloadJson, L"headersJson");
    CString strAuthType    = ExtractPayloadString(msg.m_strPayloadJson, L"authType");
    CString strAuthValue   = ExtractPayloadString(msg.m_strPayloadJson, L"authValue");

    if (strName.IsEmpty() || strBaseUrl.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"name and baseUrl are required\"}}";
    }

    CString strId;
    m_service.AddConnector(strName, strBaseUrl, strHeadersJson, strAuthType, strAuthValue, strId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"connectorId\":\"" + EscapeJson(strId) + L"\"}}";
}

CString ApiConnectorBridgeHandler::HandleRemoveConnector(const BridgeMessage& msg)
{
    CString strId = ExtractPayloadString(msg.m_strPayloadJson, L"connectorId");

    if (strId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"connectorId is required\"}}";
    }

    m_service.RemoveConnector(strId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"connectorId\":\"" + EscapeJson(strId) + L"\"}}";
}

CString ApiConnectorBridgeHandler::HandleUpdateConnector(const BridgeMessage& msg)
{
    CString strId          = ExtractPayloadString(msg.m_strPayloadJson, L"connectorId");
    CString strName        = ExtractPayloadString(msg.m_strPayloadJson, L"name");
    CString strBaseUrl     = ExtractPayloadString(msg.m_strPayloadJson, L"baseUrl");
    CString strHeadersJson = ExtractPayloadString(msg.m_strPayloadJson, L"headersJson");
    CString strAuthType    = ExtractPayloadString(msg.m_strPayloadJson, L"authType");
    CString strAuthValue   = ExtractPayloadString(msg.m_strPayloadJson, L"authValue");

    if (strId.IsEmpty() || strName.IsEmpty() || strBaseUrl.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"connectorId, name and baseUrl are required\"}}";
    }

    m_service.UpdateConnector(strId, strName, strBaseUrl, strHeadersJson, strAuthType, strAuthValue);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"connectorId\":\"" + EscapeJson(strId) + L"\"}}";
}

CString ApiConnectorBridgeHandler::HandleTestConnector(const BridgeMessage& msg)
{
    CString strId = ExtractPayloadString(msg.m_strPayloadJson, L"connectorId");

    if (strId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"connectorId is required\"}}";
    }

    CString strError;
    BOOL bOk = m_service.TestConnector(strId, strError);

    if (!bOk)
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"TEST_FAILED\",\"message\":\"" + EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{}}";
}

CString ApiConnectorBridgeHandler::SerializeConnector(const ApiConnector& conn) const
{
    CString strJson;
    strJson.Format(
        L"{"
        L"\"connectorId\":\"%s\","
        L"\"name\":\"%s\","
        L"\"baseUrl\":\"%s\","
        L"\"headersJson\":\"%s\","
        L"\"authType\":\"%s\","
        L"\"enabled\":%s,"
        L"\"createdAt\":\"%s\""
        L"}",
        (LPCWSTR)EscapeJson(conn.m_strConnectorId),
        (LPCWSTR)EscapeJson(conn.m_strName),
        (LPCWSTR)EscapeJson(conn.m_strBaseUrl),
        (LPCWSTR)EscapeJson(conn.m_strHeadersJson),
        (LPCWSTR)EscapeJson(conn.m_strAuthType),
        conn.m_bEnabled ? L"true" : L"false",
        (LPCWSTR)EscapeJson(conn.m_strCreatedAt)
    );
    return strJson;
}

CString ApiConnectorBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
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

    size_t nQuoteClose = json.find('"', nQuoteOpen + 1);
    if (nQuoteClose == std::string::npos) return L"";

    return Utf8ToWide(json.substr(nQuoteOpen + 1, nQuoteClose - nQuoteOpen - 1));
}

CString ApiConnectorBridgeHandler::EscapeJson(const CString& str) const
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
