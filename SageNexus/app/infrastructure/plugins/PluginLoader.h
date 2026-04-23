#pragma once
#include "pch.h"
#include "app/domain/interfaces/IPlugin.h"

struct DllPluginEntry
{
    HMODULE  m_hModule;
    IPlugin* m_pPlugin;
    CString  m_strDllPath;

    DllPluginEntry() : m_hModule(NULL), m_pPlugin(nullptr) {}
};

class PluginLoader
{
public:
    PluginLoader();

    BOOL LoadFromFile(const CString& strDllPath, DllPluginEntry& outEntry, CString& strError);
    void Unload(DllPluginEntry& entry);

private:
    BOOL CheckAbiVersion(HMODULE hModule, CString& strError);
};
