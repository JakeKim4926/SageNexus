#include "pch.h"
#include "app/host/MainWindow.h"
#include "resources/resource.h"
#include "app/application/SageApp.h"
#include "app/domain/model/ScheduledJob.h"
#include "Define.h"
#include <vector>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

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
    wc.hIcon         = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hIconSm       = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APP_ICON));

    if (!RegisterClassExW(&wc))
    {
        sageMgr.GetLogger().LogError(L"MainWindow: RegisterClassEx failed");
        return FALSE;
    }

    // WS_POPUP | WS_THICKFRAME: OS 타이틀바 없이 리사이즈 가능한 프레임리스 창
    // WS_EX_APPWINDOW: 작업 표시줄에 표시
    int nX = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_DEFAULT_WIDTH)  / 2;
    int nY = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_DEFAULT_HEIGHT) / 2;

    m_hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        nX, nY,
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

    case WM_WORKFLOW_PROGRESS:
        OnWorkflowProgress(static_cast<int>(wParam), static_cast<int>(lParam));
        return 0;

    case WM_WORKFLOW_COMPLETE:
        OnWorkflowComplete(static_cast<BOOL>(wParam));
        return 0;

    case WM_JOB_QUEUE_CHANGED:
        OnJobQueueChanged();
        return 0;

    case WM_TIMER:
        if (wParam == SCHEDULER_TIMER_ID)
            OnSchedulerTick();
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

    InitDwmShadow();

    m_pWebViewHost = new WebViewHost(m_hWnd);

    CString strUserDataDir = sageMgr.GetUserDataDir() + L"\\" + WEBVIEW_USER_DATA_FOLDER;
    m_pWebViewHost->Initialize(strUserDataDir);
}

void MainWindow::OnSize(int nWidth, int nHeight)
{
    if (m_pWebViewHost && m_pWebViewHost->IsReady())
    {
        m_pWebViewHost->Resize(nWidth, nHeight);

        BOOL bMaximized = IsZoomed(m_hWnd);
        CString strPayload;
        strPayload.Format(L"{\"maximized\":%s}", bMaximized ? L"true" : L"false");
        m_pWebViewHost->SendEvent(L"window:stateChanged", strPayload);
    }
}

void MainWindow::OnGetMinMaxInfo(MINMAXINFO* pInfo) const
{
    pInfo->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
    pInfo->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;

    // WS_POPUP은 기본적으로 전체 화면을 덮는다.
    // 작업 표시줄을 가리지 않도록 모니터 작업 영역 기준으로 최대화 크기를 제한한다.
    HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    if (hMonitor)
    {
        MONITORINFO mi = {};
        mi.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoW(hMonitor, &mi))
        {
            pInfo->ptMaxPosition.x = mi.rcWork.left;
            pInfo->ptMaxPosition.y = mi.rcWork.top;
            pInfo->ptMaxSize.x     = mi.rcWork.right  - mi.rcWork.left;
            pInfo->ptMaxSize.y     = mi.rcWork.bottom - mi.rcWork.top;
        }
    }
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

    RegisterBridgeHandlers();
    NavigateToShell();
    SetTimer(m_hWnd, SCHEDULER_TIMER_ID, SCHEDULER_TICK_MS, nullptr);

    RECT rcClient = {};
    GetClientRect(m_hWnd, &rcClient);
    m_pWebViewHost->Resize(rcClient.right, rcClient.bottom);
}

void MainWindow::RegisterBridgeHandlers()
{
    BridgeDispatcher& dispatcher = m_pWebViewHost->GetDispatcher();

    // 창 컨트롤 핸들러 (타이틀바 대체용)
    HWND hWnd = m_hWnd;
    dispatcher.RegisterHandler(L"window", L"minimize",
        [hWnd](const BridgeMessage&) -> CString
        {
            PostMessageW(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            return L"";
        });
    dispatcher.RegisterHandler(L"window", L"maximize",
        [hWnd](const BridgeMessage&) -> CString
        {
            PostMessageW(hWnd, WM_SYSCOMMAND, IsZoomed(hWnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
            return L"";
        });
    dispatcher.RegisterHandler(L"window", L"close",
        [hWnd](const BridgeMessage&) -> CString
        {
            PostMessageW(hWnd, WM_CLOSE, 0, 0);
            return L"";
        });

    m_importBridgeHandler.RegisterHandlers(dispatcher, m_hWnd, &m_currentTable);
    m_transformBridgeHandler.RegisterHandlers(dispatcher, &m_currentTable);
    m_exportBridgeHandler.RegisterHandlers(dispatcher, m_hWnd, &m_currentTable);
    m_historyBridgeHandler.RegisterHandlers(dispatcher);
    m_settingsBridgeHandler.RegisterHandlers(dispatcher);
    // WorkflowBridgeHandler는 JobQueueBridgeHandler를 통해 실행 — WorkflowService 인스턴스 중복 방지
    m_jobQueueBridgeHandler.RegisterHandlers(dispatcher, m_hWnd);
    m_workflowBridgeHandler.RegisterHandlers(dispatcher, m_hWnd, &m_jobQueueBridgeHandler);
    m_webExtractBridgeHandler.RegisterHandlers(dispatcher, &m_currentTable);
    m_emailBridgeHandler.RegisterHandlers(dispatcher);
    m_apiCallBridgeHandler.RegisterHandlers(dispatcher);
    m_schedulerBridgeHandler.RegisterHandlers(dispatcher);
    m_apiConnectorBridgeHandler.RegisterHandlers(dispatcher);
}

void MainWindow::NavigateToShell()
{
    m_pWebViewHost->Navigate(WEB_VIRTUAL_HOST);
}

void MainWindow::OnWorkflowProgress(int nStep, int nTotal)
{
    if (!m_pWebViewHost || !m_pWebViewHost->IsReady())
        return;

    int nPercent = (nTotal > 0) ? (nStep * 100 / nTotal) : 0;

    // 모든 실행이 JobQueue 경유이므로 JobQueueBridgeHandler에서만 조회한다.
    CString strStepName = m_jobQueueBridgeHandler.GetCurrentStepName();

    CString strPayload;
    strPayload.Format(L"{\"step\":%d,\"total\":%d,\"percent\":%d,\"stepName\":\"%s\"}",
        nStep, nTotal, nPercent, (LPCWSTR)JsonEscapeString(strStepName));
    m_pWebViewHost->SendEvent(L"workflow:progress", strPayload);
}

void MainWindow::OnWorkflowComplete(BOOL bSuccess)
{
    if (!m_pWebViewHost || !m_pWebViewHost->IsReady())
        return;

    // Workflow 성공 시 결과 DataTable을 m_currentTable에 반영 — Data Viewer에서 즉시 확인 가능
    if (bSuccess)
        UpdateCurrentTableFromWorkflow();

    CString strPayload;
    strPayload.Format(L"{\"success\":%s}", bSuccess ? L"true" : L"false");
    m_pWebViewHost->SendEvent(L"workflow:complete", strPayload);
}

void MainWindow::UpdateCurrentTableFromWorkflow()
{
    const DataTable& lastTable = m_jobQueueBridgeHandler.GetLastOutputTable();
    if (!lastTable.IsEmpty())
        m_currentTable = lastTable;
}

void MainWindow::OnJobQueueChanged()
{
    if (!m_pWebViewHost || !m_pWebViewHost->IsReady())
        return;

    m_pWebViewHost->SendEvent(L"queue:changed", L"{}");
}

void MainWindow::OnSchedulerTick()
{
    std::vector<ScheduledJob> arrDue;
    m_schedulerBridgeHandler.GetDueJobs(arrDue);

    for (int i = 0; i < (int)arrDue.size(); ++i)
    {
        CString strError;
        m_jobQueueBridgeHandler.EnqueueWorkflow(
            arrDue[i].m_strWorkflowId,
            arrDue[i].m_strWorkflowName,
            m_hWnd,
            strError);

        if (!strError.IsEmpty())
        {
            sageMgr.GetLogger().Log(LogLevel::Warning,
                L"[Scheduler] 자동 실행 실패: " + arrDue[i].m_strWorkflowId + L" / " + strError);
        }
    }
}

void MainWindow::OnDestroy()
{
    KillTimer(m_hWnd, SCHEDULER_TIMER_ID);
    sageMgr.GetLogger().LogInfo(L"MainWindow: Destroyed");
    PostQuitMessage(0);
}

// DWM 그림자 활성화: WS_POPUP 창에서 그림자가 없는 현상 방지
void MainWindow::InitDwmShadow()
{
    MARGINS margins = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(m_hWnd, &margins);
}
