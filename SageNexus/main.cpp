#include "pch.h"
#include "Define.h"
#include "app/application/SageApp.h"
#include "app/host/MainWindow.h"
#include "app/host/LoginDialog.h"

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom))
        return -1;

    if (!sageMgr.Initialize(hInstance))
    {
        CoUninitialize();
        return -1;
    }

    ProfileSecurity& security = sageMgr.GetSecurity();
    if (!security.IsCredentialsFileExists())
    {
        if (!LoginDialog::ShowSetPassword(hInstance, security))
        {
            sageMgr.Shutdown();
            CoUninitialize();
            return 0;
        }

        CString strProfilePath = sageMgr.GetAppDir() + L"\\" + PROFILE_FILE_NAME;
        CString strSignError;
        security.SignProfile(strProfilePath, strSignError);
    }
    else
    {
        if (!LoginDialog::ShowLogin(hInstance, security))
        {
            sageMgr.Shutdown();
            CoUninitialize();
            return 0;
        }
    }

    MainWindow mainWindow;
    if (!mainWindow.Create(nCmdShow))
    {
        sageMgr.Shutdown();
        CoUninitialize();
        return -1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    sageMgr.Shutdown();
    CoUninitialize();
    return static_cast<int>(msg.wParam);
}
