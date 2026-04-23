#pragma once
#include "pch.h"
#include "app/infrastructure/plugins/PluginLoader.h"

struct PluginEntry
{
    CString m_strPluginId;
    CString m_strPluginName;
};

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    void RegisterBuiltIn(const CString& strPluginId, const CString& strPluginName);
    BOOL LoadPluginsFromDirectory(const CString& strDir, CString& strErrors);

    BOOL IsEnabled(const CString& strPluginId) const;

    const std::vector<PluginEntry>& GetAllPlugins() const;
    int GetPluginCount() const;

private:
    void UnloadAllDll();

    std::vector<PluginEntry>    m_arrPlugins;
    std::vector<DllPluginEntry> m_arrDllPlugins;
    PluginLoader                m_pluginLoader;
};
