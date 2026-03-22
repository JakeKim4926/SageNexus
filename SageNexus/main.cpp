#include "pch.h"
#include "App/SageApp.h"
#include "Host/MainWindow.h"

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // WebView2는 COM STA(Single-Threaded Apartment) 초기화 필요
    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom))
        return -1;

    if (!sageMgr.Initialize(hInstance))
    {
        CoUninitialize();
        return -1;
    }

    MainWindow mainWindow;
    if (!mainWindow.Create(nCmdShow))
    {
        sageMgr.Shutdown();
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
