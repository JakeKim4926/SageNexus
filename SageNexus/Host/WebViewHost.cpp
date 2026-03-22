#include "pch.h"
#include "WebViewHost.h"
#include "../App/SageApp.h"
#include "../Define.h"

using namespace Microsoft::WRL;

WebViewHost::WebViewHost(HWND hParentWnd)
    : m_hParentWnd(hParentWnd)
    , m_eState(WebViewState::NotCreated)
    , m_pEnvironment(nullptr)
    , m_pController(nullptr)
    , m_pWebView(nullptr)
{
}

WebViewHost::~WebViewHost()
{
    if (m_pWebView)
    {
        m_pWebView->Release();
        m_pWebView = nullptr;
    }
    if (m_pController)
    {
        m_pController->Close();
        m_pController->Release();
        m_pController = nullptr;
    }
    if (m_pEnvironment)
    {
        m_pEnvironment->Release();
        m_pEnvironment = nullptr;
    }
}

void WebViewHost::Initialize(const CString& strUserDataDir)
{
    m_eState = WebViewState::Creating;
    sageMgr.GetLogger().LogInfo(L"WebViewHost: Initializing...");

    HWND hWnd = m_hParentWnd;

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        strUserDataDir,
        nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT hrResult, ICoreWebView2Environment* pEnv) -> HRESULT
            {
                OnEnvironmentCreated(hrResult, pEnv);
                return S_OK;
            }).Get());

    if (FAILED(hr))
    {
        m_eState = WebViewState::Failed;
        sageMgr.GetLogger().LogError(L"WebViewHost: CreateCoreWebView2EnvironmentWithOptions failed");
        PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 0, 0);
    }
}

void WebViewHost::OnEnvironmentCreated(HRESULT hrResult, ICoreWebView2Environment* pEnv)
{
    if (FAILED(hrResult) || !pEnv)
    {
        m_eState = WebViewState::Failed;
        sageMgr.GetLogger().LogError(L"WebViewHost: Environment creation failed");
        PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 0, 0);
        return;
    }

    pEnv->AddRef();
    m_pEnvironment = pEnv;

    HRESULT hr = pEnv->CreateCoreWebView2Controller(
        m_hParentWnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [this](HRESULT hrResult, ICoreWebView2Controller* pController) -> HRESULT
            {
                OnControllerCreated(hrResult, pController);
                return S_OK;
            }).Get());

    if (FAILED(hr))
    {
        m_eState = WebViewState::Failed;
        sageMgr.GetLogger().LogError(L"WebViewHost: CreateCoreWebView2Controller failed");
        PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 0, 0);
    }
}

void WebViewHost::OnControllerCreated(HRESULT hrResult, ICoreWebView2Controller* pController)
{
    if (FAILED(hrResult) || !pController)
    {
        m_eState = WebViewState::Failed;
        sageMgr.GetLogger().LogError(L"WebViewHost: Controller creation failed");
        PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 0, 0);
        return;
    }

    pController->AddRef();
    m_pController = pController;

    pController->get_CoreWebView2(&m_pWebView);
    if (!m_pWebView)
    {
        m_eState = WebViewState::Failed;
        sageMgr.GetLogger().LogError(L"WebViewHost: get_CoreWebView2 failed");
        PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 0, 0);
        return;
    }

    // 부모 창 크기에 맞게 초기 배치
    RECT rcClient = {};
    GetClientRect(m_hParentWnd, &rcClient);
    m_pController->put_Bounds(rcClient);

    // Web → Native 메시지 수신 등록
    m_pWebView->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this](ICoreWebView2* pSender, ICoreWebView2WebMessageReceivedEventArgs* pArgs) -> HRESULT
            {
                LPWSTR pszMessage = nullptr;
                pArgs->TryGetWebMessageAsString(&pszMessage);
                if (pszMessage)
                {
                    m_dispatcher.DispatchMessage(CString(pszMessage), m_pWebView);
                    CoTaskMemFree(pszMessage);
                }
                return S_OK;
            }).Get(),
        nullptr);

    RegisterBridgeHandlers();

    m_eState = WebViewState::Ready;
    sageMgr.GetLogger().LogInfo(L"WebViewHost: Ready");

    // 완료 알림 (WPARAM=1: 성공)
    PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 1, 0);
}

void WebViewHost::RegisterBridgeHandlers()
{
    // Phase 1: ping 핸들러 (연결 확인용)
    m_dispatcher.RegisterHandler(L"app", L"ping",
        [](const BridgeMessage& msg) -> CString
        {
            return L"{\"type\":\"response\",\"requestId\":\"" +
                   msg.m_strRequestId +
                   L"\",\"success\":true,\"payload\":{\"pong\":true}}";
        });
}

void WebViewHost::Resize(int nWidth, int nHeight)
{
    if (!m_pController)
        return;

    RECT rc = { 0, 0, nWidth, nHeight };
    m_pController->put_Bounds(rc);
}

void WebViewHost::Navigate(const CString& strUrl)
{
    if (!m_pWebView)
        return;

    m_pWebView->Navigate(strUrl);
}

BOOL WebViewHost::IsReady() const
{
    return (m_eState == WebViewState::Ready) ? TRUE : FALSE;
}

BridgeDispatcher& WebViewHost::GetDispatcher()
{
    return m_dispatcher;
}

void WebViewHost::SendEvent(const CString& strEventName, const CString& strPayloadJson)
{
    m_dispatcher.SendEvent(strEventName, strPayloadJson, m_pWebView);
}

void WebViewHost::SendSuccessResponse(const CString& strRequestId, const CString& strPayloadJson)
{
    m_dispatcher.SendSuccessResponse(strRequestId, strPayloadJson, m_pWebView);
}

void WebViewHost::SendErrorResponse(
    const CString& strRequestId,
    const CString& strErrorCode,
    const CString& strErrorMessage)
{
    m_dispatcher.SendErrorResponse(strRequestId, strErrorCode, strErrorMessage, m_pWebView);
}
