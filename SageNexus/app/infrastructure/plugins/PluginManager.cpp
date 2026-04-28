#include "pch.h"
#include "app/infrastructure/plugins/PluginManager.h"
#include "app/application/SageApp.h"
#include "app/host/BridgeDispatcher.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"

namespace
{
    constexpr LPCWSTR TAECHANG_HISTORY_TARGET_PDF = L"taechang.pdf";
    constexpr LPCWSTR TAECHANG_HISTORY_TARGET_DELIVERY = L"taechang.delivery";
    constexpr LPCWSTR TAECHANG_HISTORY_TARGET_ESTIMATE = L"taechang.estimate";
    constexpr LPCWSTR TAECHANG_HISTORY_TARGET_RECEIVABLES = L"taechang.receivables";
    constexpr LPCWSTR TAECHANG_HISTORY_ACTION_RUN_COMPARE = L"runCompare";
    constexpr LPCWSTR TAECHANG_HISTORY_ACTION_GENERATE_DELIVERY = L"generateDeliveryForms";
    constexpr LPCWSTR TAECHANG_HISTORY_ACTION_GENERATE_ESTIMATE = L"generateEstimateForms";
    constexpr LPCWSTR TAECHANG_HISTORY_ACTION_GENERATE_RECEIVABLES = L"generateReceivablesForm";
    constexpr LPCWSTR TAECHANG_HISTORY_OP_COVER_CORRECTION = L"taechang.coverCorrection";
    constexpr LPCWSTR TAECHANG_HISTORY_OP_DELIVERY = L"taechang.delivery";
    constexpr LPCWSTR TAECHANG_HISTORY_OP_ESTIMATE = L"taechang.estimate";
    constexpr LPCWSTR TAECHANG_HISTORY_OP_RECEIVABLES = L"taechang.receivables";
    constexpr int PLUGIN_HISTORY_EMPTY_COUNT = 0;

    int JsonExtractIntValue(const CString& strJson, const CString& strKey)
    {
        std::string json = WideToUtf8(strJson);
        std::string key = WideToUtf8(strKey);
        std::string token = "\"" + key + "\"";

        size_t nKeyPos = json.find(token);
        if (nKeyPos == std::string::npos)
            return PLUGIN_HISTORY_EMPTY_COUNT;

        size_t nColon = json.find(':', nKeyPos + token.size());
        if (nColon == std::string::npos)
            return PLUGIN_HISTORY_EMPTY_COUNT;

        size_t nStart = nColon + 1;
        while (nStart < json.size() && json[nStart] == ' ')
            ++nStart;

        int nSign = 1;
        if (nStart < json.size() && json[nStart] == '-')
        {
            nSign = -1;
            ++nStart;
        }

        int nValue = 0;
        BOOL bHasDigit = FALSE;
        while (nStart < json.size() && json[nStart] >= '0' && json[nStart] <= '9')
        {
            nValue = (nValue * 10) + (json[nStart] - '0');
            bHasDigit = TRUE;
            ++nStart;
        }

        return bHasDigit ? nValue * nSign : PLUGIN_HISTORY_EMPTY_COUNT;
    }

    BOOL ResolveTaechangHistoryOperation(
        const CString& strTarget,
        const CString& strAction,
        CString& outOperationType)
    {
        if (strTarget == TAECHANG_HISTORY_TARGET_PDF && strAction == TAECHANG_HISTORY_ACTION_RUN_COMPARE)
        {
            outOperationType = TAECHANG_HISTORY_OP_COVER_CORRECTION;
            return TRUE;
        }

        if (strTarget == TAECHANG_HISTORY_TARGET_DELIVERY && strAction == TAECHANG_HISTORY_ACTION_GENERATE_DELIVERY)
        {
            outOperationType = TAECHANG_HISTORY_OP_DELIVERY;
            return TRUE;
        }

        if (strTarget == TAECHANG_HISTORY_TARGET_ESTIMATE && strAction == TAECHANG_HISTORY_ACTION_GENERATE_ESTIMATE)
        {
            outOperationType = TAECHANG_HISTORY_OP_ESTIMATE;
            return TRUE;
        }

        if (strTarget == TAECHANG_HISTORY_TARGET_RECEIVABLES && strAction == TAECHANG_HISTORY_ACTION_GENERATE_RECEIVABLES)
        {
            outOperationType = TAECHANG_HISTORY_OP_RECEIVABLES;
            return TRUE;
        }

        return FALSE;
    }
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    UnloadAllDll();
}

void PluginManager::RegisterBuiltIn(const CString& strPluginId, const CString& strPluginName)
{
    PluginEntry entry;
    entry.m_strPluginId   = strPluginId;
    entry.m_strPluginName = strPluginName;
    m_arrPlugins.push_back(entry);
}

BOOL PluginManager::LoadPluginsFromDirectory(const CString& strDir, CString& strErrors)
{
    m_arrPluginPages.clear();

    CString strPattern = strDir + L"\\*.dll";
    WIN32_FIND_DATAW findData = {};
    HANDLE hFind = FindFirstFileW(strPattern, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return TRUE;

    BOOL bAllLoaded = TRUE;
    do
    {
        CString strDllPath = strDir + L"\\" + findData.cFileName;
        DllPluginEntry dllEntry;
        CString strError;

        if (!m_pluginLoader.LoadFromFile(strDllPath, dllEntry, strError))
        {
            if (!strErrors.IsEmpty())
                strErrors += L"\n";
            strErrors += strError;
            bAllLoaded = FALSE;
            continue;
        }

        PluginEntry entry;
        entry.m_strPluginId   = dllEntry.m_pPlugin->GetPluginId();
        entry.m_strPluginName = dllEntry.m_pPlugin->GetPluginName();

        m_arrDllPlugins.push_back(dllEntry);
        m_arrPlugins.push_back(entry);

        int nPageCount = dllEntry.m_pPlugin->GetPageCount();
        for (int i = 0; i < nPageCount; ++i)
        {
            PluginPageInfo pageInfo;
            if (!dllEntry.m_pPlugin->GetPageInfo(i, pageInfo))
                continue;

            CString strPageId    = pageInfo.m_pszPageId;
            CString strPageName  = pageInfo.m_pszPageName;
            CString strEntryPath = pageInfo.m_pszEntryPath;
            if (strPageId.IsEmpty() || strEntryPath.IsEmpty())
                continue;

            PluginPageEntry pageEntry;
            pageEntry.m_strPluginId  = entry.m_strPluginId;
            pageEntry.m_strPageId    = strPageId;
            pageEntry.m_strPageName  = strPageName.IsEmpty() ? entry.m_strPluginName : strPageName;
            pageEntry.m_strEntryPath = strEntryPath;

            int nSlash = dllEntry.m_strDllPath.ReverseFind(L'\\');
            if (nSlash >= 0)
                pageEntry.m_strBaseDir = dllEntry.m_strDllPath.Left(nSlash);

            m_arrPluginPages.push_back(pageEntry);
        }
    }
    while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return bAllLoaded;
}

BOOL PluginManager::IsEnabled(const CString& strPluginId) const
{
    return sageMgr.GetProfile().IsPluginEnabled(strPluginId);
}

void PluginManager::RegisterPluginBridgeHandlers(BridgeDispatcher& dispatcher)
{
    for (int i = 0; i < static_cast<int>(m_arrDllPlugins.size()); ++i)
    {
        IPlugin* pPlugin = m_arrDllPlugins[i].m_pPlugin;
        if (pPlugin == nullptr)
            continue;

        int nCommandCount = pPlugin->GetCommandCount();
        for (int j = 0; j < nCommandCount; ++j)
        {
            PluginCommandInfo commandInfo;
            if (!pPlugin->GetCommandInfo(j, commandInfo))
                continue;

            CString strTarget = commandInfo.m_pszTarget;
            CString strAction = commandInfo.m_pszAction;
            if (strTarget.IsEmpty() || strAction.IsEmpty())
                continue;

            // 파일/폴더 다이얼로그를 여는 커맨드는 WebView2 이벤트 콜백 밖에서 실행해야 한다
            CString strActionLower = strAction;
            strActionLower.MakeLower();
            BOOL bDeferred = (strActionLower.Find(L"dialog") >= 0 ||
                              strActionLower.Find(L"folder") >= 0);

            BridgeCommandHandler handler =
                [this, pPlugin, strTarget, strAction](const BridgeMessage& msg) -> CString
                {
                    CString strResponseJson;
                    CString strError;

                    if (!pPlugin->HandleCommand(
                            strTarget,
                            strAction,
                            msg.m_strRequestId,
                            msg.m_strPayloadJson,
                            strResponseJson,
                            strError))
                    {
                        CString strPluginError = strError.IsEmpty() ? CString(L"Plugin command failed.") : strError;
                        CString strJson;
                        strJson.Format(
                            L"{\"type\":\"response\",\"requestId\":\"%s\",\"success\":false,"
                            L"\"error\":{\"code\":\"SNX_PLUGIN_001\",\"message\":\"%s\"}}",
                            (LPCWSTR)msg.m_strRequestId,
                            (LPCWSTR)JsonEscapeString(strPluginError));
                        SavePluginExecutionRecord(strTarget, strAction, msg.m_strPayloadJson, strJson);
                        return strJson;
                    }

                    if (strResponseJson.IsEmpty())
                    {
                        strResponseJson.Format(
                            L"{\"type\":\"response\",\"requestId\":\"%s\",\"success\":true,\"payload\":{}}",
                            (LPCWSTR)msg.m_strRequestId);
                    }

                    SavePluginExecutionRecord(strTarget, strAction, msg.m_strPayloadJson, strResponseJson);
                    return strResponseJson;
                };

            if (bDeferred)
                dispatcher.RegisterDeferredHandler(strTarget, strAction, handler);
            else
                dispatcher.RegisterHandler(strTarget, strAction, handler);
        }
    }
}

const std::vector<PluginPageEntry>& PluginManager::GetPluginPages() const
{
    return m_arrPluginPages;
}

BOOL PluginManager::ResolvePluginWebFile(
    const CString& strPluginId,
    const CString& strRelativePath,
    CString& outFilePath,
    CString& strError) const
{
    CString strBaseDir;
    for (int i = 0; i < static_cast<int>(m_arrPluginPages.size()); ++i)
    {
        if (m_arrPluginPages[i].m_strPluginId == strPluginId)
        {
            strBaseDir = m_arrPluginPages[i].m_strBaseDir;
            break;
        }
    }

    if (strBaseDir.IsEmpty())
    {
        strError = L"Plugin web root not found.";
        return FALSE;
    }

    CString strRelative = strRelativePath;
    strRelative.Replace(L"/", L"\\");
    while (!strRelative.IsEmpty() && strRelative[0] == L'\\')
        strRelative = strRelative.Mid(1);

    if (strRelative.Find(L"..") >= 0)
    {
        strError = L"Invalid plugin web path.";
        return FALSE;
    }

    CString strCandidate = strBaseDir + L"\\" + strRelative;
    wchar_t szResolvedBase[MAX_PATH] = {};
    wchar_t szResolvedPath[MAX_PATH] = {};

    if (GetFullPathNameW(strBaseDir, MAX_PATH, szResolvedBase, nullptr) == 0 ||
        GetFullPathNameW(strCandidate, MAX_PATH, szResolvedPath, nullptr) == 0)
    {
        strError = L"Failed to resolve plugin web path.";
        return FALSE;
    }

    CString strResolvedBase = szResolvedBase;
    CString strResolvedPath = szResolvedPath;
    CString strBasePrefix = strResolvedBase;
    if (strBasePrefix.Right(1) != L"\\")
        strBasePrefix += L"\\";

    if (strResolvedPath.Left(strBasePrefix.GetLength()).CompareNoCase(strBasePrefix) != 0)
    {
        strError = L"Plugin web path escaped base directory.";
        return FALSE;
    }

    DWORD dwAttr = GetFileAttributesW(strResolvedPath);
    if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        strError = L"Plugin web file not found.";
        return FALSE;
    }

    outFilePath = strResolvedPath;
    return TRUE;
}

const std::vector<PluginEntry>& PluginManager::GetAllPlugins() const
{
    return m_arrPlugins;
}

int PluginManager::GetPluginCount() const
{
    return static_cast<int>(m_arrPlugins.size());
}

void PluginManager::UnloadAllDll()
{
    for (DllPluginEntry& entry : m_arrDllPlugins)
        m_pluginLoader.Unload(entry);

    m_arrDllPlugins.clear();
    m_arrPluginPages.clear();
}

void PluginManager::SavePluginExecutionRecord(
    const CString& strTarget,
    const CString& strAction,
    const CString& strPayloadJson,
    const CString& strResponseJson) const
{
    CString strOperationType;
    if (!ResolveTaechangHistoryOperation(strTarget, strAction, strOperationType))
        return;

    ExecutionRecord record;
    record.m_strOperationType = strOperationType;
    record.m_strSourceName = JsonExtractString(strPayloadJson, L"inputPath");
    if (record.m_strSourceName.IsEmpty())
        record.m_strSourceName = JsonExtractString(strPayloadJson, L"filePath");
    if (record.m_strSourceName.IsEmpty())
        record.m_strSourceName = strOperationType;

    record.m_nRowCount = JsonExtractIntValue(strResponseJson, L"rowCount");
    if (record.m_nRowCount <= PLUGIN_HISTORY_EMPTY_COUNT)
        record.m_nRowCount = JsonExtractIntValue(strResponseJson, L"generatedCount");
    record.m_nColumnCount = PLUGIN_HISTORY_EMPTY_COUNT;
    record.m_strOutputPath = JsonExtractString(strResponseJson, L"filePath");
    record.m_bSuccess = JsonExtractBool(strResponseJson, L"success");
    if (!record.m_bSuccess)
        record.m_strErrorMessage = JsonExtractString(strResponseJson, L"message");

    ExecutionHistoryStore historyStore;
    CString strHistoryError;
    historyStore.SaveRecord(record, strHistoryError);
}
