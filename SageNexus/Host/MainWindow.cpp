#include "../pch.h"
#include "MainWindow.h"
#include "../App/SageApp.h"
#include "../Define.h"

MainWindow::MainWindow()
    : m_hWnd(nullptr)
    , m_pWebViewHost(nullptr)
{
}

MainWindow::~MainWindow()
{
    delete m_pWebViewHost;
    m_pWebViewHost = nullptr;
}

BOOL MainWindow::Create(int nCmdShow)
{
    HINSTANCE hInst = sageMgr.GetHInstance();

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIcon         = LoadIconW(hInst, IDI_APPLICATION);
    wc.hIconSm       = LoadIconW(hInst, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        sageMgr.GetLogger().LogError(L"MainWindow: RegisterClassEx failed");
        return FALSE;
    }

    m_hWnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT,
        nullptr, nullptr, hInst, this);

    if (!m_hWnd)
    {
        sageMgr.GetLogger().LogError(L"MainWindow: CreateWindowEx failed");
        return FALSE;
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return TRUE;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    }
    else
    {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pThis)
        return pThis->HandleMessage(uMsg, wParam, lParam);

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        OnCreate();
        return 0;

    case WM_SIZE:
        OnSize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_GETMINMAXINFO:
        OnGetMinMaxInfo(reinterpret_cast<MINMAXINFO*>(lParam));
        return 0;

    case WM_WEBVIEW_READY:
        OnWebViewReady(static_cast<BOOL>(wParam));
        return 0;

    case WM_DESTROY:
        OnDestroy();
        return 0;
    }

    return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
}

void MainWindow::OnCreate()
{
    sageMgr.GetLogger().LogInfo(L"MainWindow: Created");

    m_pWebViewHost = new WebViewHost(m_hWnd);

    CString strUserDataDir = sageMgr.GetAppDir() + L"\\" + WEBVIEW_USER_DATA_FOLDER;
    m_pWebViewHost->Initialize(strUserDataDir);
}

void MainWindow::OnSize(int nWidth, int nHeight)
{
    if (m_pWebViewHost && m_pWebViewHost->IsReady())
        m_pWebViewHost->Resize(nWidth, nHeight);
}

void MainWindow::OnGetMinMaxInfo(MINMAXINFO* pInfo) const
{
    pInfo->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
    pInfo->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;
}

void MainWindow::OnWebViewReady(BOOL bSuccess)
{
    if (!bSuccess)
    {
        sageMgr.GetLogger().LogError(L"MainWindow: WebView2 initialization failed");
        MessageBoxW(m_hWnd,
            L"WebView2 초기화에 실패했습니다.\nMicrosoft Edge WebView2 Runtime이 설치되어 있는지 확인하세요.",
            WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return;
    }

    sageMgr.GetLogger().LogInfo(L"MainWindow: WebView2 ready, navigating to shell");

    NavigateToShell();

    // WebView2 로드 후 창 크기 동기화
    RECT rcClient = {};
    GetClientRect(m_hWnd, &rcClient);
    m_pWebViewHost->Resize(rcClient.right, rcClient.bottom);
}

void MainWindow::NavigateToShell()
{
    CString strUrl = L"file:///" + sageMgr.GetAppDir() + L"\\" + WEB_ENTRY_FILE;
    // 경로 구분자를 URL 형식으로 변환
    strUrl.Replace(L'\\', L'/');
    m_pWebViewHost->Navigate(strUrl);

    // 네비게이션 완료 후 appReady 이벤트는 JavaScript 로드 완료 시점에 발송
    // NavigationCompleted 이벤트 등록은 Phase 1에서 WM_WEBVIEW_READY 이후 처리
}

void MainWindow::OnDestroy()
{
    sageMgr.GetLogger().LogInfo(L"MainWindow: Destroyed");
    PostQuitMessage(0);
}
