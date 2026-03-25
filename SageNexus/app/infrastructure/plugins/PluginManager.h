#pragma once
#include "pch.h"

struct PluginEntry
{
    CString m_strPluginId;
    CString m_strPluginName;
};

class PluginManager
{
public:
    PluginManager();

    void RegisterBuiltIn(const CString& strPluginId, const CString& strPluginName);

    BOOL IsEnabled(const CString& strPluginId) const;

    const std::vector<PluginEntry>& GetAllPlugins() const;
    int GetPluginCount() const;

private:
    std::vector<PluginEntry> m_arrPlugins;
};
