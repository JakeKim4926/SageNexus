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

    CString strPayload = L"{\"filePath\":\"" + JsonEscapeString(strFilePath) + L"\"}";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strPayload + L"}";
}

CString ImportBridgeHandler::HandleLoadFile(const BridgeMessage& msg)
{
    CString strFilePath = JsonExtractString(msg.m_strPayloadJson, L"filePath");

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
               JsonEscapeString(strError) + L"\"}}";
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
    json += (LPCWSTR)JsonEscapeString(table.GetSourceName());
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
        json += (LPCWSTR)JsonEscapeString(col.m_strInternalName);
        json += L"\",\"displayNameKo\":\"";
        json += (LPCWSTR)JsonEscapeString(col.m_strDisplayNameKo);
        json += L"\",\"displayNameEn\":\"";
        json += (LPCWSTR)JsonEscapeString(col.m_strDisplayNameEn);
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
            json += (LPCWSTR)JsonEscapeString(row.m_arrCells[j]);
            json += L"\"";
        }
        json += L"]";
    }
    json += L"]";

    json += L"}";
    return CString(json.c_str());
}
