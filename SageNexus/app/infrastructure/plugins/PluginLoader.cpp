#include "pch.h"
#include "app/infrastructure/plugins/PluginLoader.h"
#include "Define.h"

typedef IPlugin* (*FnSagePlugin_Create)();
typedef void     (*FnSagePlugin_Destroy)(IPlugin*);
typedef int      (*FnSagePlugin_GetAbiVersion)();

PluginLoader::PluginLoader()
{
}

BOOL PluginLoader::LoadFromFile(const CString& strDllPath, DllPluginEntry& outEntry, CString& strError)
{
    HMODULE hModule = LoadLibraryExW(strDllPath, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (hModule == NULL)
    {
        strError.Format(L"LoadLibrary failed: %s (error %u)", strDllPath.GetString(), GetLastError());
        return FALSE;
    }

    if (!CheckAbiVersion(hModule, strError))
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    FnSagePlugin_Create fnCreate = reinterpret_cast<FnSagePlugin_Create>(
        GetProcAddress(hModule, "SagePlugin_Create"));

    if (fnCreate == nullptr)
    {
        strError.Format(L"SagePlugin_Create not found: %s", strDllPath.GetString());
        FreeLibrary(hModule);
        return FALSE;
    }

    IPlugin* pPlugin = fnCreate();
    if (pPlugin == nullptr)
    {
        strError.Format(L"SagePlugin_Create returned null: %s", strDllPath.GetString());
        FreeLibrary(hModule);
        return FALSE;
    }

    outEntry.m_hModule    = hModule;
    outEntry.m_pPlugin    = pPlugin;
    outEntry.m_strDllPath = strDllPath;
    return TRUE;
}

void PluginLoader::Unload(DllPluginEntry& entry)
{
    if (entry.m_hModule == NULL)
        return;

    FnSagePlugin_Destroy fnDestroy = reinterpret_cast<FnSagePlugin_Destroy>(
        GetProcAddress(entry.m_hModule, "SagePlugin_Destroy"));

    if (fnDestroy != nullptr && entry.m_pPlugin != nullptr)
        fnDestroy(entry.m_pPlugin);

    FreeLibrary(entry.m_hModule);
    entry.m_hModule = NULL;
    entry.m_pPlugin = nullptr;
}

BOOL PluginLoader::CheckAbiVersion(HMODULE hModule, CString& strError)
{
    FnSagePlugin_GetAbiVersion fnGetVersion = reinterpret_cast<FnSagePlugin_GetAbiVersion>(
        GetProcAddress(hModule, "SagePlugin_GetAbiVersion"));

    if (fnGetVersion == nullptr)
    {
        strError = L"SagePlugin_GetAbiVersion not found in DLL";
        return FALSE;
    }

    int nVersion = fnGetVersion();
    if (nVersion != PLUGIN_ABI_VERSION)
    {
        strError.Format(L"Plugin ABI version mismatch: expected %d, got %d", PLUGIN_ABI_VERSION, nVersion);
        return FALSE;
    }

    return TRUE;
}
