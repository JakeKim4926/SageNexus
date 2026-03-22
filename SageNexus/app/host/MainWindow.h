#pragma once
#include "pch.h"
#include "WebViewHost.h"

// Win32 메인 창
// WebView2 호스팅, 창 리사이즈, 메시지 처리를 담당한다.
// 비즈니스 로직 수행 금지.
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
    void SendAppReadyEvent();

    HWND          m_hWnd;
    WebViewHost*  m_pWebViewHost;
};
