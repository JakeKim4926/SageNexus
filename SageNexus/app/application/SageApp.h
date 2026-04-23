#pragma once
#include "pch.h"
#include "app/infrastructure/logging/FileLogger.h"
#include "app/infrastructure/config/JsonConfigStore.h"
#include "app/domain/model/SolutionProfile.h"
#include "app/infrastructure/plugins/PluginManager.h"
#include "app/infrastructure/security/ProfileSecurity.h"

#define sageMgr SageApp::GetInstance()

class SageApp
{
public:
    static SageApp& GetInstance();

    BOOL Initialize(HINSTANCE hInstance);
    void Shutdown();

    HINSTANCE       GetHInstance() const;
    const CString&  GetAppDir() const;      // 설치 폴더 (exe 위치, 읽기 전용 리소스)
    const CString&  GetUserDataDir() const; // 사용자 데이터 폴더 (%APPDATA%/SageNexus)
    const CString&  GetDataDir() const;     // 데이터 폴더 (UserDataDir/Data)
    const CString&  GetLogDir() const;      // 로그 폴더 (UserDataDir/Logs)
    const CString&  GetProfilePath() const; // 설치 폴더 profile.json

    FileLogger&       GetLogger();
    JsonConfigStore&  GetConfigStore();
    SolutionProfile&  GetProfile();
    PluginManager&    GetPluginManager();
    ProfileSecurity&  GetSecurity();
    BOOL              ReloadProfile(CString& strError);
    BOOL              SaveProfile(CString& strError);

private:
    SageApp();
    ~SageApp();
    SageApp(const SageApp&)            = delete;
    SageApp& operator=(const SageApp&) = delete;

    BOOL InitializePaths();
    BOOL LoadProfile(CString& strError);
    void ReleaseResources();

    HINSTANCE        m_hInstance;
    CString          m_strAppDir;
    CString          m_strUserDataDir;
    CString          m_strDataDir;
    CString          m_strLogDir;
    CString          m_strProfilePath;
    BOOL             m_bInitialized;

    FileLogger*      m_pLogger;
    JsonConfigStore* m_pConfigStore;
    SolutionProfile  m_profile;
    PluginManager    m_pluginManager;
    ProfileSecurity  m_security;
};
