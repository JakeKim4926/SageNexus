#pragma once
#include "pch.h"
#include "app/infrastructure/plugins/PluginLoader.h"

struct PluginEntry
{
    CString m_strPluginId;
    CString m_strPluginName;
};

struct PluginPageEntry
{
    CString m_strPluginId;
    CString m_strPageId;
    CString m_strPageName;
    CString m_strEntryPath;
    CString m_strBaseDir;
};

class BridgeDispatcher;

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    void RegisterBuiltIn(const CString& strPluginId, const CString& strPluginName);
    BOOL LoadPluginsFromDirectory(const CString& strDir, CString& strErrors);

    BOOL IsEnabled(const CString& strPluginId) const;
    void RegisterPluginBridgeHandlers(BridgeDispatcher& dispatcher);
    const std::vector<PluginPageEntry>& GetPluginPages() const;
    BOOL ResolvePluginWebFile(
        const CString& strPluginId,
        const CString& strRelativePath,
        CString& outFilePath,
        CString& strError) const;

    const std::vector<PluginEntry>& GetAllPlugins() const;
    int GetPluginCount() const;

private:
    void UnloadAllDll();
    void SavePluginExecutionRecord(
        const CString& strTarget,
        const CString& strAction,
        const CString& strPayloadJson,
        const CString& strResponseJson) const;

    std::vector<PluginEntry>    m_arrPlugins;
    std::vector<DllPluginEntry> m_arrDllPlugins;
    std::vector<PluginPageEntry> m_arrPluginPages;
    PluginLoader                m_pluginLoader;
};
