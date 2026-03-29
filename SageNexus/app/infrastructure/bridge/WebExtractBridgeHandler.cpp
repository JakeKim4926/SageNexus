#include "pch.h"
#include "app/infrastructure/bridge/WebExtractBridgeHandler.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "Define.h"

WebExtractBridgeHandler::WebExtractBridgeHandler()
    : m_pCurrentTable(nullptr)
{
}

void WebExtractBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, DataTable* pCurrentTable)
{
    m_pCurrentTable = pCurrentTable;

    dispatcher.RegisterHandler(L"webExtract", L"fetchAndExtract",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleFetchAndExtract(msg);
        });
}

CString WebExtractBridgeHandler::HandleFetchAndExtract(const BridgeMessage& msg)
{
    CString strUrl      = ExtractPayloadString(msg.m_strPayloadJson, L"url");
    CString strSelector = ExtractPayloadString(msg.m_strPayloadJson, L"selector");

    if (strUrl.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_WEB_001\",\"message\":\"URL이 비어 있습니다.\"}}";
    }

    DataTable table;
    CString strError;

    if (!m_service.FetchAndExtract(strUrl, strSelector, table, strError))
    {
        ExecutionHistoryStore historyStore;
        ExecutionRecord record;
        record.m_strOperationType = L"webExtract";
        record.m_strSourceName    = strUrl;
        record.m_bSuccess         = FALSE;
        record.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(record, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_WEB_002\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    *m_pCurrentTable = table;

    ExecutionHistoryStore historyStore;
    ExecutionRecord record;
    record.m_strOperationType = L"webExtract";
    record.m_strSourceName    = strUrl;
    record.m_nRowCount        = m_pCurrentTable->GetRowCount();
    record.m_nColumnCount     = m_pCurrentTable->GetColumnCount();
    record.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(record, strHistError);

    std::wstring json;
    json.reserve(64 * 1024);
    json += L"{";
    json += L"\"tableId\":\"web-1\",";
    json += L"\"sourceName\":\"";
    json += (LPCWSTR)EscapeJson(m_pCurrentTable->GetSourceName());
    json += L"\",";

    wchar_t buf[32];
    swprintf_s(buf, 32, L"%d", m_pCurrentTable->GetRowCount());
    json += L"\"rowCount\":";
    json += buf;
    json += L",";

    swprintf_s(buf, 32, L"%d", m_pCurrentTable->GetColumnCount());
    json += L"\"columnCount\":";
    json += buf;
    json += L",";

    json += L"\"columns\":[";
    for (int i = 0; i < m_pCurrentTable->GetColumnCount(); ++i)
    {
        if (i > 0) json += L",";
        const DataColumn& col = m_pCurrentTable->GetColumn(i);
        json += L"{\"internalName\":\"";
        json += (LPCWSTR)EscapeJson(col.m_strInternalName);
        json += L"\",\"displayNameKo\":\"";
        json += (LPCWSTR)EscapeJson(col.m_strDisplayNameKo);
        json += L"\",\"displayNameEn\":\"";
        json += (LPCWSTR)EscapeJson(col.m_strDisplayNameEn);
        json += L"\"}";
    }
    json += L"],";

    int nMaxRows = m_pCurrentTable->GetRowCount();
    if (nMaxRows > CSV_MAX_PREVIEW_ROWS)
        nMaxRows = CSV_MAX_PREVIEW_ROWS;

    json += L"\"rows\":[";
    for (int i = 0; i < nMaxRows; ++i)
    {
        if (i > 0) json += L",";
        const DataRow& row = m_pCurrentTable->GetRow(i);
        json += L"[";
        for (int j = 0; j < (int)row.m_arrCells.size(); ++j)
        {
            if (j > 0) json += L",";
            json += L"\"";
            json += (LPCWSTR)EscapeJson(row.m_arrCells[j]);
            json += L"\"";
        }
        json += L"]";
    }
    json += L"]";
    json += L"}";

    CString strPayload(json.c_str());
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strPayload + L"}";
}

CString WebExtractBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0)
        return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0)
        return L"";

    int nStart = nColon + 1;
    while (nStart < strJson.GetLength() && strJson[nStart] == L' ')
        ++nStart;

    if (nStart >= strJson.GetLength() || strJson[nStart] != L'"')
        return L"";

    int nEnd = nStart + 1;
    while (nEnd < strJson.GetLength())
    {
        if (strJson[nEnd] == L'\\') { nEnd += 2; continue; }
        if (strJson[nEnd] == L'"')  break;
        ++nEnd;
    }

    if (nEnd >= strJson.GetLength())
        return L"";

    return strJson.Mid(nStart + 1, nEnd - nStart - 1);
}

CString WebExtractBridgeHandler::EscapeJson(const CString& str) const
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
