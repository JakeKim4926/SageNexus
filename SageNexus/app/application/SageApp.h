#pragma once
#include "pch.h"
#include "app/infrastructure/logging/FileLogger.h"
#include "app/infrastructure/config/JsonConfigStore.h"
#include "app/domain/model/SolutionProfile.h"

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
