#include "pch.h"
#include "app/infrastructure/bridge/ExportBridgeHandler.h"
#include "app/application/services/ExportService.h"
#include <commdlg.h>

ExportBridgeHandler::ExportBridgeHandler()
    : m_hParentWnd(nullptr)
    , m_pSharedTable(nullptr)
{
}

void ExportBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, HWND hParentWnd, DataTable* pSharedTable)
{
    m_hParentWnd  = hParentWnd;
    m_pSharedTable = pSharedTable;

    dispatcher.RegisterHandler(L"artifact.export", L"exportCsv",
        [this, hParentWnd](const BridgeMessage& msg) -> CString
        {
            return HandleExportCsv(msg, hParentWnd);
        });
}

CString ExportBridgeHandler::HandleExportCsv(const BridgeMessage& msg, HWND hParentWnd)
{
    if (!m_pSharedTable || m_pSharedTable->IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_001\",\"message\":\"내보낼 데이터가 없습니다.\"}}";
    }

    wchar_t szFile[MAX_PATH] = {};

    CString strSuggest = m_pSharedTable->GetSourceName();
    int nDot = strSuggest.ReverseFind(L'.');
    if (nDot >= 0)
        strSuggest = strSuggest.Left(nDot);
    strSuggest += L"_export.csv";

    if (strSuggest.GetLength() < MAX_PATH)
        wcsncpy_s(szFile, MAX_PATH, strSuggest, _TRUNCATE);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize   = sizeof(ofn);
    ofn.hwndOwner     = hParentWnd;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrFilter   = L"CSV 파일\0*.csv\0모든 파일\0*.*\0";
    ofn.lpstrTitle    = L"다른 이름으로 저장";
    ofn.lpstrDefExt   = L"csv";
    ofn.Flags         = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileNameW(&ofn))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_002\",\"message\":\"저장이 취소되었습니다.\"}}";
    }

    CString strFilePath = szFile;
    ExportService exportService;
    CString strError;

    if (!exportService.ExportToCsv(*m_pSharedTable, strFilePath, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_003\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"filePath\":\"" +
           EscapeJsonString(strFilePath) + L"\"}}";
}

CString ExportBridgeHandler::EscapeJsonString(const CString& str) const
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
