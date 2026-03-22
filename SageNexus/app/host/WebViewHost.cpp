#include "pch.h"
#include "app/host/WebViewHost.h"
#include "app/application/SageApp.h"
#include "app/domain/model/SolutionProfile.h"
#include "Define.h"

using namespace Microsoft::WRL;

WebViewHost::WebViewHost(HWND hParentWnd)
    : m_hParentWnd(hParentWnd)
    , m_eState(WebViewState::NotCreated)
    , m_bAppReadySent(FALSE)
    , m_pEnvironment(nullptr)
    , m_pController(nullptr)
    , m_pWebView(nullptr)
    , m_tokenNavigationCompleted({})
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

    RECT rcClient = {};
    GetClientRect(m_hParentWnd, &rcClient);
    m_pController->put_Bounds(rcClient);

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

    PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 1, 0);
}

void WebViewHost::RegisterBridgeHandlers()
{
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

    m_pWebView->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [this](ICoreWebView2* pSender, ICoreWebView2NavigationCompletedEventArgs* pArgs) -> HRESULT
            {
                if (m_bAppReadySent)
                    return S_OK;

                BOOL bSuccess = FALSE;
                pArgs->get_IsSuccess(&bSuccess);
                if (!bSuccess)
                {
                    sageMgr.GetLogger().LogError(L"WebViewHost: NavigationCompleted with failure");
                    return S_OK;
                }

                m_bAppReadySent = TRUE;

                const SolutionProfile& profile = sageMgr.GetProfile();
                CString strPayload;
                strPayload.Format(
                    L"{\"profileName\":\"%s\",\"interfaceLanguage\":\"%s\",\"outputLanguage\":\"%s\"}",
                    (LPCWSTR)profile.GetProfileName(),
                    (LPCWSTR)profile.GetDefaultInterfaceLanguage(),
                    (LPCWSTR)profile.GetDefaultOutputLanguage());

                m_dispatcher.SendEvent(L"appReady", strPayload, m_pWebView);
                sageMgr.GetLogger().LogInfo(L"WebViewHost: appReady event sent");
                return S_OK;
            }).Get(),
        &m_tokenNavigationCompleted);

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
