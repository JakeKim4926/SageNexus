#include "pch.h"
#include "app/application/SageApp.h"
#include "Define.h"

SageApp& SageApp::GetInstance()
{
    static SageApp instance;
    return instance;
}

SageApp::SageApp()
    : m_hInstance(nullptr)
    , m_bInitialized(FALSE)
    , m_pLogger(nullptr)
    , m_pConfigStore(nullptr)
{
}

SageApp::~SageApp()
{
    delete m_pConfigStore;
    m_pConfigStore = nullptr;

    delete m_pLogger;
    m_pLogger = nullptr;
}

BOOL SageApp::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    if (!InitializePaths())
        return FALSE;

    m_pLogger = new FileLogger(m_strLogDir);
    if (!m_pLogger->Initialize())
        return FALSE;

    m_pConfigStore = new JsonConfigStore(m_strAppDir);
    m_pConfigStore->Load();

    m_profile.SetDefault();

    CString strProfilePath = m_strAppDir + L"\\" + PROFILE_FILE_NAME;
    CString strProfileError;
    if (!m_profile.LoadFromFile(strProfilePath, strProfileError))
    {
        m_pLogger->LogInfo(L"Profile file not found, creating default: " + strProfilePath);
        WriteDefaultProfileFile(strProfilePath);
    }
    else
    {
        m_pLogger->LogInfo(L"Profile loaded: " + m_profile.GetProfileName());
    }

    m_pluginManager.RegisterBuiltIn(L"import",    L"데이터 가져오기");
    m_pluginManager.RegisterBuiltIn(L"transform", L"데이터 변환");
    m_pluginManager.RegisterBuiltIn(L"export",    L"내보내기");
    m_pluginManager.RegisterBuiltIn(L"history",   L"실행 이력");

    m_bInitialized = TRUE;
    m_pLogger->LogInfo(L"SageApp initialized");
    return TRUE;
}

void SageApp::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_pLogger)
        m_pLogger->LogInfo(L"SageApp shutting down");

    if (m_pConfigStore)
        m_pConfigStore->Save();

    if (m_pLogger)
        m_pLogger->Flush();

    m_bInitialized = FALSE;
}

BOOL SageApp::InitializePaths()
{
    wchar_t szPath[MAX_PATH] = {};
    if (!GetModuleFileNameW(m_hInstance, szPath, MAX_PATH))
        return FALSE;

    m_strAppDir = szPath;
    int nSlash = m_strAppDir.ReverseFind(L'\\');
    if (nSlash < 0)
        return FALSE;

    m_strAppDir  = m_strAppDir.Left(nSlash);
    m_strDataDir = m_strAppDir + L"\\" + DATA_DIR_NAME;
    m_strLogDir  = m_strAppDir + L"\\" + LOG_DIR_NAME;

    CreateDirectoryW(m_strDataDir, nullptr);
    CreateDirectoryW(m_strLogDir,  nullptr);
    return TRUE;
}

void SageApp::WriteDefaultProfileFile(const CString& strFilePath) const
{
    std::string strPath = WideToUtf8(strFilePath);
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return;

    file << "{\n";
    file << "  \"profileId\": \"default\",\n";
    file << "  \"profileName\": \"Default Profile\",\n";
    file << "  \"defaultInterfaceLanguage\": \"ko\",\n";
    file << "  \"defaultOutputLanguage\": \"ko\",\n";
    file << "  \"showDataViewer\": true,\n";
    file << "  \"showTransform\": true,\n";
    file << "  \"showExport\": true,\n";
    file << "  \"showHistory\": true,\n";
    file << "  \"showSettings\": true,\n";
    file << "  \"plugin_import\": true,\n";
    file << "  \"plugin_transform\": true,\n";
    file << "  \"plugin_export\": true,\n";
    file << "  \"plugin_history\": true\n";
    file << "}\n";
}

HINSTANCE       SageApp::GetHInstance() const  { return m_hInstance;    }
const CString&  SageApp::GetAppDir() const     { return m_strAppDir;    }
const CString&  SageApp::GetDataDir() const    { return m_strDataDir;   }
const CString&  SageApp::GetLogDir() const     { return m_strLogDir;    }
FileLogger&     SageApp::GetLogger()           { return *m_pLogger;     }
JsonConfigStore& SageApp::GetConfigStore()     { return *m_pConfigStore;}
SolutionProfile& SageApp::GetProfile()         { return m_profile;       }
PluginManager&   SageApp::GetPluginManager()   { return m_pluginManager; }
