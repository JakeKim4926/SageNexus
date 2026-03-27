#include "pch.h"
#include "app/infrastructure/bridge/ImportBridgeHandler.h"
#include "app/application/services/ImportService.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "Define.h"
#include <commdlg.h>

ImportBridgeHandler::ImportBridgeHandler()
    : m_pSharedTable(nullptr)
    , m_nTableIdCounter(0)
{
}

void ImportBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, HWND hParentWnd, DataTable* pSharedTable)
{
    m_pSharedTable = pSharedTable;

    dispatcher.RegisterHandler(L"data.import", L"openFileDialog",
        [this, hParentWnd](const BridgeMessage& msg) -> CString
        {
            return HandleOpenFileDialog(msg, hParentWnd);
        });

    dispatcher.RegisterHandler(L"data.import", L"loadFile",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleLoadFile(msg);
        });
}

CString ImportBridgeHandler::HandleOpenFileDialog(const BridgeMessage& msg, HWND hParentWnd)
{
    wchar_t szFile[MAX_PATH] = {};

    OPENFILENAMEW ofn = {};
    ofn.lStructSize   = sizeof(ofn);
    ofn.hwndOwner     = hParentWnd;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrFilter   = L"지원 파일\0*.csv;*.xlsx\0CSV 파일\0*.csv\0Excel 파일\0*.xlsx\0모든 파일\0*.*\0";
    ofn.lpstrTitle    = L"파일 선택";
    ofn.Flags         = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    CString strFilePath;
    if (GetOpenFileNameW(&ofn))
        strFilePath = szFile;

    CString strPayload = L"{\"filePath\":\"" + EscapeJsonString(strFilePath) + L"\"}";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strPayload + L"}";
}

CString ImportBridgeHandler::HandleLoadFile(const BridgeMessage& msg)
{
    CString strFilePath = UnescapeJsonString(ExtractPayloadField(msg.m_strPayloadJson, L"filePath"));

    if (strFilePath.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_FILE_001\",\"message\":\"파일 경로가 비어 있습니다.\"}}";
    }

    ++m_nTableIdCounter;
    CString strTableId;
    strTableId.Format(L"tbl-%d", m_nTableIdCounter);

    ImportService importService;
    DataTable table;
    CString strError;

    if (!importService.LoadFromFile(strFilePath, table, strError))
    {
        int nSlash = strFilePath.ReverseFind(L'\\');
        CString strFileName = (nSlash >= 0) ? strFilePath.Mid(nSlash + 1) : strFilePath;

        ExecutionHistoryStore historyStore;
        ExecutionRecord histRecord;
        histRecord.m_strOperationType = L"import";
        histRecord.m_strSourceName    = strFileName;
        histRecord.m_bSuccess         = FALSE;
        histRecord.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(histRecord, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_FILE_002\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    *m_pSharedTable = table;

    ExecutionHistoryStore historyStore;
    ExecutionRecord histRecord;
    histRecord.m_strOperationType = L"import";
    histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
    histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
    histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
    histRecord.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(histRecord, strHistError);

    CString strTableJson = SerializeTableToJson(*m_pSharedTable, strTableId);
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strTableJson + L"}";
}

CString ImportBridgeHandler::SerializeTableToJson(const DataTable& table, const CString& strTableId) const
{
    std::wstring json;
    json.reserve(64 * 1024);
    json += L"{";

    json += L"\"tableId\":\"";
    json += (LPCWSTR)strTableId;
    json += L"\",";

    json += L"\"sourceName\":\"";
    json += (LPCWSTR)EscapeJsonString(table.GetSourceName());
    json += L"\",";

    wchar_t buf[32];
    swprintf_s(buf, 32, L"%d", table.GetRowCount());
    json += L"\"rowCount\":";
    json += buf;
    json += L",";

    swprintf_s(buf, 32, L"%d", table.GetColumnCount());
    json += L"\"columnCount\":";
    json += buf;
    json += L",";

    json += L"\"columns\":[";
    for (int i = 0; i < table.GetColumnCount(); ++i)
    {
        if (i > 0) json += L",";
        const DataColumn& col = table.GetColumn(i);
        json += L"{\"internalName\":\"";
        json += (LPCWSTR)EscapeJsonString(col.m_strInternalName);
        json += L"\",\"displayNameKo\":\"";
        json += (LPCWSTR)EscapeJsonString(col.m_strDisplayNameKo);
        json += L"\",\"displayNameEn\":\"";
        json += (LPCWSTR)EscapeJsonString(col.m_strDisplayNameEn);
        json += L"\"}";
    }
    json += L"],";

    int nMaxRows = table.GetRowCount();
    if (nMaxRows > CSV_MAX_PREVIEW_ROWS)
        nMaxRows = CSV_MAX_PREVIEW_ROWS;

    json += L"\"rows\":[";
    for (int i = 0; i < nMaxRows; ++i)
    {
        if (i > 0) json += L",";
        const DataRow& row = table.GetRow(i);
        json += L"[";
        for (int j = 0; j < (int)row.m_arrCells.size(); ++j)
        {
            if (j > 0) json += L",";
            json += L"\"";
            json += (LPCWSTR)EscapeJsonString(row.m_arrCells[j]);
            json += L"\"";
        }
        json += L"]";
    }
    json += L"]";

    json += L"}";
    return CString(json.c_str());
}

CString ImportBridgeHandler::EscapeJsonString(const CString& str) const
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

CString ImportBridgeHandler::UnescapeJsonString(const CString& str) const
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        if (str[i] == L'\\' && i + 1 < str.GetLength())
        {
            ++i;
            switch (str[i])
            {
            case L'"':  strResult += L'"';  break;
            case L'\\': strResult += L'\\'; break;
            case L'/':  strResult += L'/';  break;
            case L'n':  strResult += L'\n'; break;
            case L'r':  strResult += L'\r'; break;
            case L't':  strResult += L'\t'; break;
            default:    strResult += str[i]; break;
            }
        }
        else
        {
            strResult += str[i];
        }
    }
    return strResult;
}

CString ImportBridgeHandler::ExtractPayloadField(const CString& strPayloadJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strPayloadJson.Find(strSearch);
    if (nPos < 0)
        return L"";

    int nColon = strPayloadJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0)
        return L"";

    int nValueStart = nColon + 1;
    while (nValueStart < strPayloadJson.GetLength() && strPayloadJson[nValueStart] == L' ')
        ++nValueStart;

    if (nValueStart >= strPayloadJson.GetLength() || strPayloadJson[nValueStart] != L'"')
        return L"";

    int nEnd = nValueStart + 1;
    while (nEnd < strPayloadJson.GetLength())
    {
        if (strPayloadJson[nEnd] == L'\\')
        {
            nEnd += 2;
            continue;
        }
        if (strPayloadJson[nEnd] == L'"')
            break;
        ++nEnd;
    }

    if (nEnd >= strPayloadJson.GetLength())
        return L"";

    return strPayloadJson.Mid(nValueStart + 1, nEnd - nValueStart - 1);
}
