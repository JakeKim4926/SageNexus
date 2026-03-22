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

    if (!sageMgr.Initialize(hInstance))
        return -1;

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
    return static_cast<int>(msg.wParam);
}
