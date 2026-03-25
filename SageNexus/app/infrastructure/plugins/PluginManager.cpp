#include "pch.h"
#include "app/infrastructure/plugins/PluginManager.h"
#include "app/application/SageApp.h"

PluginManager::PluginManager()
{
}

void PluginManager::RegisterBuiltIn(const CString& strPluginId, const CString& strPluginName)
{
    PluginEntry entry;
    entry.m_strPluginId   = strPluginId;
    entry.m_strPluginName = strPluginName;
    m_arrPlugins.push_back(entry);
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
