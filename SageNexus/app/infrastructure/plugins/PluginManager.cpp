#include "pch.h"
#include "app/infrastructure/plugins/PluginManager.h"
#include "app/application/SageApp.h"

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
    }
    while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return bAllLoaded;
}

BOOL PluginManager::IsEnabled(const CString& strPluginId) const
{
    return sageMgr.GetProfile().IsPluginEnabled(strPluginId);
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
}
