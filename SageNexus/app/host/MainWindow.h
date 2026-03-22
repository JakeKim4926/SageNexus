#pragma once
#include "pch.h"
#include "WebViewHost.h"
#include "app/infrastructure/bridge/ImportBridgeHandler.h"

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    BOOL Create(int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnCreate();
    void OnSize(int nWidth, int nHeight);
    void OnGetMinMaxInfo(MINMAXINFO* pInfo) const;
    void OnWebViewReady(BOOL bSuccess);
    void OnDestroy();

    void NavigateToShell();
    void RegisterBridgeHandlers();

    HWND                 m_hWnd;
    WebViewHost*         m_pWebViewHost;
    ImportBridgeHandler  m_importBridgeHandler;
};
