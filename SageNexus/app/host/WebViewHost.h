#pragma once
#include "pch.h"
#include "EnumDefine.h"
#include "BridgeDispatcher.h"

// WebView2 생성/소멸/리사이즈/메시지 수신을 담당한다.
// UI 스레드에서만 사용한다.
class WebViewHost
{
public:
    explicit WebViewHost(HWND hParentWnd);
    ~WebViewHost();

    // 비동기 초기화. 완료 시 hParentWnd에 WM_WEBVIEW_READY 전송
    void Initialize(const CString& strUserDataDir);

    void Resize(int nWidth, int nHeight);
    void Navigate(const CString& strUrl);

    BOOL IsReady() const;

    BridgeDispatcher& GetDispatcher();

    // WebView2로 메시지 전송
    void SendEvent(const CString& strEventName, const CString& strPayloadJson);
    void SendSuccessResponse(const CString& strRequestId, const CString& strPayloadJson);
    void SendErrorResponse(const CString& strRequestId,
                           const CString& strErrorCode,
                           const CString& strErrorMessage);

private:
    void OnEnvironmentCreated(HRESULT hrResult, ICoreWebView2Environment* pEnv);
    void OnControllerCreated(HRESULT hrResult, ICoreWebView2Controller* pController);
    void RegisterWebResourceHandler();
    void RegisterBridgeHandlers();

    HWND                    m_hParentWnd;
    WebViewState            m_eState;
    BOOL                    m_bAppReadySent; // appReady 이벤트를 한 번만 보내기 위한 플래그

    ICoreWebView2Environment* m_pEnvironment;
    ICoreWebView2Controller*  m_pController;
    ICoreWebView2*            m_pWebView;

    EventRegistrationToken m_tokenNavigationCompleted;
    EventRegistrationToken m_tokenWebResourceRequested;
    BridgeDispatcher m_dispatcher;
};
