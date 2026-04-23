#include "pch.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include "resources/resource.h"

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
    Shutdown();
}

BOOL SageApp::Initialize(HINSTANCE hInstance)
{
    if (m_bInitialized)
        return FALSE;

    m_hInstance = hInstance;

    if (!InitializePaths())
        return FALSE;

    m_pLogger = new FileLogger(m_strLogDir);
    if (!m_pLogger->Initialize())
    {
        ReleaseResources();
        return FALSE;
    }

    CString strCredPath = m_strUserDataDir + L"\\" + CREDENTIALS_FILE_NAME;
    m_security.SetCredentialsPath(strCredPath);

    m_pConfigStore = new JsonConfigStore(m_strUserDataDir);
    m_pConfigStore->Load();

    CString strProfileError;
    if (!LoadProfile(strProfileError))
    {
        m_pLogger->LogError(L"Profile load failed: " + strProfileError);
        ReleaseResources();
        return FALSE;
    }

    m_pLogger->LogInfo(L"Profile loaded: " + m_profile.GetProfileName());

    m_pluginManager.RegisterBuiltIn(L"import",     L"데이터 가져오기");
    m_pluginManager.RegisterBuiltIn(L"transform",  L"데이터 변환");
    m_pluginManager.RegisterBuiltIn(L"export",     L"내보내기");
    m_pluginManager.RegisterBuiltIn(L"history",    L"실행 이력");
    m_pluginManager.RegisterBuiltIn(L"workflow",   L"워크플로우");
    m_pluginManager.RegisterBuiltIn(L"webextract", L"웹 추출");

    CString strPluginsDir = m_strAppDir + L"\\" + PLUGINS_DIR_NAME;
    CString strPluginErrors;
    m_pluginManager.LoadPluginsFromDirectory(strPluginsDir, strPluginErrors);
    if (!strPluginErrors.IsEmpty())
        m_pLogger->LogWarning(L"Some plugins failed to load:\n" + strPluginErrors);

    m_bInitialized = TRUE;
    m_pLogger->LogInfo(L"SageApp initialized");
    return TRUE;
}

void SageApp::Shutdown()
{
    if (!m_bInitialized && m_pLogger == nullptr)
        return;

    if (m_pLogger)
        m_pLogger->LogInfo(L"SageApp shutting down");

    if (m_pConfigStore)
        m_pConfigStore->Save();

    if (m_pLogger)
        m_pLogger->Flush();

    ReleaseResources();

    m_hInstance = nullptr;
    m_strAppDir.Empty();
    m_strUserDataDir.Empty();
    m_strDataDir.Empty();
    m_strLogDir.Empty();
    m_bInitialized = FALSE;
}

void SageApp::ReleaseResources()
{
    delete m_pConfigStore;
    m_pConfigStore = nullptr;

    delete m_pLogger;
    m_pLogger = nullptr;
}

BOOL SageApp::InitializePaths()
{
    // 설치 폴더: exe 위치 (WebView2Loader.dll)
    wchar_t szExePath[MAX_PATH] = {};
    if (!GetModuleFileNameW(m_hInstance, szExePath, MAX_PATH))
        return FALSE;

    m_strAppDir = szExePath;
    int nSlash = m_strAppDir.ReverseFind(L'\\');
    if (nSlash < 0)
        return FALSE;
    m_strAppDir = m_strAppDir.Left(nSlash);

    // 사용자 데이터 폴더: %APPDATA%\SageNexus\ (settings, logs, data, WebViewData)
    wchar_t szAppData[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szAppData)))
        return FALSE;

    m_strUserDataDir = CString(szAppData) + L"\\" + USER_DATA_APP_NAME;
    m_strDataDir     = m_strUserDataDir + L"\\" + DATA_DIR_NAME;
    m_strLogDir      = m_strUserDataDir + L"\\" + LOG_DIR_NAME;
    m_strProfilePath = m_strAppDir + L"\\" + PROFILE_FILE_NAME;

    CreateDirectoryW(m_strUserDataDir, nullptr);
    CreateDirectoryW(m_strDataDir, nullptr);
    CreateDirectoryW(m_strLogDir, nullptr);
    return TRUE;
}

HINSTANCE SageApp::GetHInstance() const
{
    return m_hInstance;
}

const CString& SageApp::GetAppDir() const
{
    return m_strAppDir;
}

const CString& SageApp::GetUserDataDir() const
{
    return m_strUserDataDir;
}

const CString& SageApp::GetDataDir() const
{
    return m_strDataDir;
}

const CString& SageApp::GetLogDir() const
{
    return m_strLogDir;
}

const CString& SageApp::GetProfilePath() const
{
    return m_strProfilePath;
}

FileLogger& SageApp::GetLogger()
{
    assert(m_pLogger != nullptr);
    return *m_pLogger;
}

JsonConfigStore& SageApp::GetConfigStore()
{
    assert(m_pConfigStore != nullptr);
    return *m_pConfigStore;
}

SolutionProfile& SageApp::GetProfile()
{
    return m_profile;
}

PluginManager& SageApp::GetPluginManager()
{
    return m_pluginManager;
}

ProfileSecurity& SageApp::GetSecurity()
{
    return m_security;
}

BOOL SageApp::ReloadProfile(CString& strError)
{
    return LoadProfile(strError);
}

BOOL SageApp::SaveProfile(CString& strError)
{
    return m_profile.SaveToFile(m_strProfilePath, strError);
}

BOOL SageApp::LoadProfile(CString& strError)
{
    m_profile.SetDefault();

    DWORD dwAttr = GetFileAttributesW(m_strProfilePath);
    if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return m_profile.LoadFromFile(m_strProfilePath, strError);
    }

    if (!m_profile.LoadFromResource(IDR_PROFILE_JSON, strError))
        return FALSE;

    CString strSaveError;
    if (!m_profile.SaveToFile(m_strProfilePath, strSaveError) && m_pLogger != nullptr)
        m_pLogger->LogWarning(L"profile.json bootstrap skipped: " + strSaveError);

    return TRUE;
}
