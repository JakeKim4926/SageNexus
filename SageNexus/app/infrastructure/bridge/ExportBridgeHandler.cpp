#include "pch.h"
#include "app/infrastructure/bridge/ExportBridgeHandler.h"
#include "app/application/services/ExportService.h"
#include "app/application/SageApp.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "app/infrastructure/history/ArtifactStore.h"
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

    dispatcher.RegisterHandler(L"artifact.export", L"exportXlsx",
        [this, hParentWnd](const BridgeMessage& msg) -> CString
        {
            return HandleExportXlsx(msg, hParentWnd);
        });

    dispatcher.RegisterHandler(L"artifact.export", L"exportHtml",
        [this, hParentWnd](const BridgeMessage& msg) -> CString
        {
            return HandleExportHtml(msg, hParentWnd);
        });

    dispatcher.RegisterHandler(L"artifact.export", L"getArtifacts",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetArtifacts(msg);
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
    CString strLang = sageMgr.GetConfigStore().GetString(L"outputLanguage", L"ko");
    ExportService exportService;
    CString strError;

    if (!exportService.ExportToCsv(*m_pSharedTable, strFilePath, strLang, strError))
    {
        ExecutionHistoryStore historyStore;
        ExecutionRecord histRecord;
        histRecord.m_strOperationType = L"export";
        histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
        histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
        histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
        histRecord.m_bSuccess         = FALSE;
        histRecord.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(histRecord, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_003\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    ExecutionHistoryStore historyStore;
    ExecutionRecord histRecord;
    histRecord.m_strOperationType = L"export";
    histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
    histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
    histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
    histRecord.m_strOutputPath    = strFilePath;
    histRecord.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(histRecord, strHistError);

    ArtifactStore artifactStore;
    Artifact artifact;
    artifact.m_strSourceName  = m_pSharedTable->GetSourceName();
    artifact.m_strFilePath    = strFilePath;
    artifact.m_strFormat      = L"csv";
    artifact.m_nRowCount      = m_pSharedTable->GetRowCount();
    artifact.m_nColumnCount   = m_pSharedTable->GetColumnCount();
    CString strArtError;
    artifactStore.SaveArtifact(artifact, strArtError);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"filePath\":\"" +
           EscapeJsonString(strFilePath) + L"\"}}";
}

CString ExportBridgeHandler::HandleExportXlsx(const BridgeMessage& msg, HWND hParentWnd)
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
    strSuggest += L"_export.xlsx";

    if (strSuggest.GetLength() < MAX_PATH)
        wcsncpy_s(szFile, MAX_PATH, strSuggest, _TRUNCATE);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize   = sizeof(ofn);
    ofn.hwndOwner     = hParentWnd;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrFilter   = L"XLSX 파일\0*.xlsx\0모든 파일\0*.*\0";
    ofn.lpstrTitle    = L"다른 이름으로 저장";
    ofn.lpstrDefExt   = L"xlsx";
    ofn.Flags         = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileNameW(&ofn))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_002\",\"message\":\"저장이 취소되었습니다.\"}}";
    }

    CString strFilePath = szFile;
    CString strLang = sageMgr.GetConfigStore().GetString(L"outputLanguage", L"ko");
    ExportService exportService;
    CString strError;

    if (!exportService.ExportToXlsx(*m_pSharedTable, strFilePath, strLang, strError))
    {
        ExecutionHistoryStore historyStore;
        ExecutionRecord histRecord;
        histRecord.m_strOperationType = L"export";
        histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
        histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
        histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
        histRecord.m_bSuccess         = FALSE;
        histRecord.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(histRecord, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_003\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    ExecutionHistoryStore historyStore;
    ExecutionRecord histRecord;
    histRecord.m_strOperationType = L"export";
    histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
    histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
    histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
    histRecord.m_strOutputPath    = strFilePath;
    histRecord.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(histRecord, strHistError);

    ArtifactStore artifactStore;
    Artifact artifact;
    artifact.m_strSourceName  = m_pSharedTable->GetSourceName();
    artifact.m_strFilePath    = strFilePath;
    artifact.m_strFormat      = L"xlsx";
    artifact.m_nRowCount      = m_pSharedTable->GetRowCount();
    artifact.m_nColumnCount   = m_pSharedTable->GetColumnCount();
    CString strArtError;
    artifactStore.SaveArtifact(artifact, strArtError);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"filePath\":\"" +
           EscapeJsonString(strFilePath) + L"\"}}";
}

CString ExportBridgeHandler::HandleExportHtml(const BridgeMessage& msg, HWND hParentWnd)
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
    strSuggest += L"_report.html";

    if (strSuggest.GetLength() < MAX_PATH)
        wcsncpy_s(szFile, MAX_PATH, strSuggest, _TRUNCATE);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize   = sizeof(ofn);
    ofn.hwndOwner     = hParentWnd;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrFilter   = L"HTML 파일\0*.html\0모든 파일\0*.*\0";
    ofn.lpstrTitle    = L"다른 이름으로 저장";
    ofn.lpstrDefExt   = L"html";
    ofn.Flags         = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileNameW(&ofn))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_002\",\"message\":\"저장이 취소되었습니다.\"}}";
    }

    CString strFilePath = szFile;
    CString strLang = sageMgr.GetConfigStore().GetString(L"outputLanguage", L"ko");
    ExportService exportService;
    CString strError;

    if (!exportService.ExportToHtml(*m_pSharedTable, strFilePath, strLang, strError))
    {
        ExecutionHistoryStore historyStore;
        ExecutionRecord histRecord;
        histRecord.m_strOperationType = L"export";
        histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
        histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
        histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
        histRecord.m_bSuccess         = FALSE;
        histRecord.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(histRecord, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_003\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    ExecutionHistoryStore historyStore;
    ExecutionRecord histRecord;
    histRecord.m_strOperationType = L"export";
    histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
    histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
    histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
    histRecord.m_strOutputPath    = strFilePath;
    histRecord.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(histRecord, strHistError);

    ArtifactStore artifactStore;
    Artifact artifact;
    artifact.m_strSourceName  = m_pSharedTable->GetSourceName();
    artifact.m_strFilePath    = strFilePath;
    artifact.m_strFormat      = L"html";
    artifact.m_nRowCount      = m_pSharedTable->GetRowCount();
    artifact.m_nColumnCount   = m_pSharedTable->GetColumnCount();
    CString strArtError;
    artifactStore.SaveArtifact(artifact, strArtError);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"filePath\":\"" +
           EscapeJsonString(strFilePath) + L"\"}}";
}

CString ExportBridgeHandler::HandleGetArtifacts(const BridgeMessage& msg)
{
    ArtifactStore store;
    std::vector<Artifact> arrArtifacts;
    CString strError;

    if (!store.LoadArtifacts(arrArtifacts, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_EX_010\",\"message\":\"" +
               EscapeJsonString(strError) + L"\"}}";
    }

    CString strItems;
    for (int i = (int)arrArtifacts.size() - 1; i >= 0; --i)
    {
        const Artifact& art = arrArtifacts[i];
        if (!strItems.IsEmpty()) strItems += L",";

        wchar_t szRows[16] = {};
        wchar_t szCols[16] = {};
        _itow_s(art.m_nRowCount,    szRows, 16, 10);
        _itow_s(art.m_nColumnCount, szCols, 16, 10);

        CString strItem = L"{";
        strItem += L"\"id\":\""         + EscapeJsonString(art.m_strId)         + L"\",";
        strItem += L"\"sourceName\":\"" + EscapeJsonString(art.m_strSourceName) + L"\",";
        strItem += L"\"filePath\":\""   + EscapeJsonString(art.m_strFilePath)   + L"\",";
        strItem += L"\"format\":\""     + EscapeJsonString(art.m_strFormat)     + L"\",";
        strItem += L"\"createdAt\":\""  + EscapeJsonString(art.m_strCreatedAt)  + L"\",";
        strItem += L"\"rowCount\":";    strItem += szRows; strItem += L",";
        strItem += L"\"columnCount\":"; strItem += szCols;
        strItem += L"}";
        strItems += strItem;
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"artifacts\":[" + strItems + L"]}}";
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
