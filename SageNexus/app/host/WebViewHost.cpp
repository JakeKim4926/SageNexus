#include "pch.h"
#include "app/host/WebViewHost.h"
#include "app/application/SageApp.h"
#include "app/domain/model/SolutionProfile.h"
#include "Define.h"
#include "resources/resource.h"

using namespace Microsoft::WRL;

WebViewHost::WebViewHost(HWND hParentWnd)
    : m_hParentWnd(hParentWnd)
    , m_eState(WebViewState::NotCreated)
    , m_bAppReadySent(FALSE)
    , m_pEnvironment(nullptr)
    , m_pController(nullptr)
    , m_pWebView(nullptr)
    , m_tokenNavigationCompleted({})
    , m_tokenWebResourceRequested({})
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

    // CSS -webkit-app-region: drag 지원 활성화 (타이틀바 대체 드래그 영역)
    // 반드시 첫 Navigate() 전에 설정해야 적용된다
    ICoreWebView2Settings* pSettings = nullptr;
    if (SUCCEEDED(m_pWebView->get_Settings(&pSettings)) && pSettings)
    {
        ICoreWebView2Settings9* pSettings9 = nullptr;
        if (SUCCEEDED(pSettings->QueryInterface(IID_ICoreWebView2Settings9,
                                                reinterpret_cast<void**>(&pSettings9)))
            && pSettings9)
        {
            pSettings9->put_IsNonClientRegionSupportEnabled(TRUE);
            pSettings9->Release();
        }
        pSettings->Release();
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

    RegisterWebResourceHandler();
    RegisterBridgeHandlers();

    m_eState = WebViewState::Ready;
    sageMgr.GetLogger().LogInfo(L"WebViewHost: Ready");

    PostMessageW(m_hParentWnd, WM_WEBVIEW_READY, 1, 0);
}

void WebViewHost::RegisterWebResourceHandler()
{
    // Newer WebView2 runtimes deprecate the 2-arg filter for script/stylesheet
    // subresources. Use _22 with explicit RequestSourceKinds when available.
    ICoreWebView2_22* pWv22 = nullptr;
    HRESULT hrQI = m_pWebView->QueryInterface(
        IID_ICoreWebView2_22, reinterpret_cast<void**>(&pWv22));
    if (SUCCEEDED(hrQI) && pWv22)
    {
        pWv22->AddWebResourceRequestedFilterWithRequestSourceKinds(
            L"https://app.sagenexus/*",
            COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL,
            COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL);
        pWv22->Release();
        pWv22 = nullptr;
    }
    else
    {
        m_pWebView->AddWebResourceRequestedFilter(
            L"https://app.sagenexus/*",
            COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    }

    ICoreWebView2Environment* pEnv = m_pEnvironment;

    m_pWebView->add_WebResourceRequested(
        Callback<ICoreWebView2WebResourceRequestedEventHandler>(
            [pEnv](ICoreWebView2* /*pSender*/,
                   ICoreWebView2WebResourceRequestedEventArgs* pArgs) -> HRESULT
            {
                ICoreWebView2WebResourceRequest* pReq = nullptr;
                pArgs->get_Request(&pReq);
                if (!pReq)
                    return S_OK;

                LPWSTR pszUri = nullptr;
                pReq->get_Uri(&pszUri);
                pReq->Release();
                if (!pszUri)
                    return S_OK;

                CString strUri(pszUri);
                CoTaskMemFree(pszUri);

                int            nResId  = 0;
                const wchar_t* pszMime = L"application/octet-stream";

                if (strUri == L"https://app.sagenexus/index.html")
                {
                    nResId  = IDR_WEBUI_INDEX_HTML;
                    pszMime = L"text/html; charset=utf-8";
                }
                else if (strUri == L"https://app.sagenexus/src/styles/styles.css")
                {
                    nResId  = IDR_WEBUI_STYLES_CSS;
                    pszMime = L"text/css; charset=utf-8";
                }
                else if (strUri == L"https://app.sagenexus/src/core/bridge/bridge.js")
                {
                    nResId  = IDR_WEBUI_BRIDGE_JS;
                    pszMime = L"application/javascript; charset=utf-8";
                }
                else if (strUri == L"https://app.sagenexus/resources/app.ico")
                {
                    nResId  = IDR_APP_ICO;
                    pszMime = L"image/x-icon";
                }

                if (nResId == 0)
                    return S_OK;

                HMODULE hMod    = GetModuleHandleW(nullptr);
                HRSRC   hRes    = FindResourceW(hMod, MAKEINTRESOURCEW(nResId), RT_RCDATA);
                if (!hRes)
                    return S_OK;

                HGLOBAL hGlobal = LoadResource(hMod, hRes);
                if (!hGlobal)
                    return S_OK;

                void*  pData  = LockResource(hGlobal);
                DWORD  dwSize = SizeofResource(hMod, hRes);
                if (!pData || dwSize == 0)
                    return S_OK;

                IStream* pStream = nullptr;
                if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &pStream)))
                    return S_OK;

                ULONG nWritten = 0;
                pStream->Write(pData, dwSize, &nWritten);
                LARGE_INTEGER liZero = {};
                pStream->Seek(liZero, STREAM_SEEK_SET, nullptr);

                CString strHeaders;
                strHeaders.Format(L"Content-Type: %s", pszMime);

                ICoreWebView2WebResourceResponse* pResponse = nullptr;
                pEnv->CreateWebResourceResponse(pStream, 200, L"OK", strHeaders, &pResponse);
                pStream->Release();

                if (pResponse)
                {
                    pArgs->put_Response(pResponse);
                    pResponse->Release();
                }
                return S_OK;
            }).Get(),
        &m_tokenWebResourceRequested);
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
                const MenuVisibility&  vis     = profile.GetMenuVisibility();
                CString strPayload;
                strPayload.Format(
                    L"{"
                    L"\"profileName\":\"%s\","
                    L"\"interfaceLanguage\":\"%s\","
                    L"\"outputLanguage\":\"%s\","
                    L"\"menuVisibility\":{"
                    L"\"showDataViewer\":%s,"
                    L"\"showTransform\":%s,"
                    L"\"showExport\":%s,"
                    L"\"showHistory\":%s,"
                    L"\"showWorkflow\":%s,"
                    L"\"showWebextract\":%s,"
                    L"\"showSettings\":%s"
                    L"}}",
                    (LPCWSTR)profile.GetProfileName(),
                    (LPCWSTR)profile.GetDefaultInterfaceLanguage(),
                    (LPCWSTR)profile.GetDefaultOutputLanguage(),
                    vis.m_bShowDataViewer  ? L"true" : L"false",
                    vis.m_bShowTransform   ? L"true" : L"false",
                    vis.m_bShowExport      ? L"true" : L"false",
                    vis.m_bShowHistory     ? L"true" : L"false",
                    vis.m_bShowWorkflow    ? L"true" : L"false",
                    vis.m_bShowWebextract  ? L"true" : L"false",
                    vis.m_bShowSettings    ? L"true" : L"false");

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
