#include "pch.h"
#include "app/host/MainWindow.h"
#include "app/application/SageApp.h"
#include "app/domain/model/ScheduledJob.h"
#include "Define.h"
#include <vector>
#include <dwmapi.h>
#include <windowsx.h>
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

    case WM_NCCALCSIZE:
        return OnNcCalcSize(wParam, lParam);

    case WM_NCHITTEST:
        return OnNcHitTest(lParam);

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
    CString strUrl = L"file:///" + sageMgr.GetAppDir() + L"\\" + WEB_ENTRY_FILE;
    strUrl.Replace(L'\\', L'/');
    m_pWebViewHost->Navigate(strUrl);
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

// 타이틀바 제거: WM_NCCALCSIZE에서 0 반환 → 전체 창 영역을 클라이언트 영역으로 확장
// 최대화 시 화면 밖으로 나가는 프레임 오프셋을 보정한다
LRESULT MainWindow::OnNcCalcSize(WPARAM wParam, LPARAM lParam)
{
    if (wParam == TRUE)
    {
        if (IsZoomed(m_hWnd))
        {
            NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
            int nFrame = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
            pParams->rgrc[0].left   += nFrame;
            pParams->rgrc[0].right  -= nFrame;
            pParams->rgrc[0].top    += nFrame;
            pParams->rgrc[0].bottom -= nFrame;
        }
        return 0;
    }
    return DefWindowProcW(m_hWnd, WM_NCCALCSIZE, wParam, lParam);
}

// 타이틀바 제거 후 리사이즈 핸들을 직접 반환한다
// 최대화/최소화 상태에서는 리사이즈가 불필요하므로 HTCLIENT 반환
LRESULT MainWindow::OnNcHitTest(LPARAM lParam)
{
    if (IsZoomed(m_hWnd) || IsIconic(m_hWnd))
        return HTCLIENT;

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    RECT rcWin;
    GetWindowRect(m_hWnd, &rcWin);

    int nBorder = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

    BOOL bLeft   = pt.x <  rcWin.left   + nBorder;
    BOOL bRight  = pt.x >= rcWin.right  - nBorder;
    BOOL bTop    = pt.y <  rcWin.top    + nBorder;
    BOOL bBottom = pt.y >= rcWin.bottom - nBorder;

    if (bTop    && bLeft)  return HTTOPLEFT;
    if (bTop    && bRight) return HTTOPRIGHT;
    if (bBottom && bLeft)  return HTBOTTOMLEFT;
    if (bBottom && bRight) return HTBOTTOMRIGHT;
    if (bTop)              return HTTOP;
    if (bBottom)           return HTBOTTOM;
    if (bLeft)             return HTLEFT;
    if (bRight)            return HTRIGHT;

    return HTCLIENT;
}

// DWM 그림자 활성화: 타이틀바 제거 후 그림자가 사라지는 현상 방지
void MainWindow::InitDwmShadow()
{
    MARGINS margins = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(m_hWnd, &margins);
}
