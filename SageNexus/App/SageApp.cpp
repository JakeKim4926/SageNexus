#include "pch.h"
#include "SageApp.h"
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

HINSTANCE       SageApp::GetHInstance() const  { return m_hInstance;    }
const CString&  SageApp::GetAppDir() const     { return m_strAppDir;    }
const CString&  SageApp::GetDataDir() const    { return m_strDataDir;   }
const CString&  SageApp::GetLogDir() const     { return m_strLogDir;    }
FileLogger&     SageApp::GetLogger()           { return *m_pLogger;     }
JsonConfigStore& SageApp::GetConfigStore()     { return *m_pConfigStore;}
SolutionProfile& SageApp::GetProfile()         { return m_profile;      }
