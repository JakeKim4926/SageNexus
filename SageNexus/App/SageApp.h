#pragma once
#include "pch.h"
#include "../Domain/SolutionProfile.h"

class FileLogger;
class JsonConfigStore;

// 앱 공통 인프라 서비스 싱글톤
// 로거, 설정, 공통 경로, 프로필에 접근하는 단일 진입점
#define sageMgr SageApp::GetInstance()

class SageApp
{
public:
    static SageApp& GetInstance();

    BOOL Initialize(HINSTANCE hInstance);
    void Shutdown();

    HINSTANCE       GetHInstance() const;
    const CString&  GetAppDir() const;
    const CString&  GetDataDir() const;
    const CString&  GetLogDir() const;

    FileLogger&          GetLogger();
    JsonConfigStore&     GetConfigStore();
    SolutionProfile&     GetProfile();

private:
    SageApp();
    ~SageApp();
    SageApp(const SageApp&)            = delete;
    SageApp& operator=(const SageApp&) = delete;

    BOOL InitializePaths();

    HINSTANCE        m_hInstance;
    CString          m_strAppDir;
    CString          m_strDataDir;
    CString          m_strLogDir;
    BOOL             m_bInitialized;

    FileLogger*       m_pLogger;
    JsonConfigStore*  m_pConfigStore;
    SolutionProfile   m_profile;
};
