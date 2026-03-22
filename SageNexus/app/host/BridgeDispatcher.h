#pragma once
#include "pch.h"
#include "EnumDefine.h"

// 파싱된 브릿지 메시지 구조
struct BridgeMessage
{
    BridgeMessageType m_eType;      // command / response / event
    CString           m_strRequestId;
    CString           m_strTarget;  // command 대상 (e.g. "navigation", "data")
    CString           m_strAction;  // command 액션 (e.g. "navigate", "load")
    CString           m_strName;    // event 이름
    CString           m_strPayloadJson; // payload 원문 JSON 문자열

    BridgeMessage()
        : m_eType(BridgeMessageType::Unknown)
    {}
};

// command 핸들러 타입: target + action 에 대응
// 핸들러는 payload JSON 문자열을 받아 응답 JSON 문자열을 반환한다
// 반환값이 비어있으면 기본 success 응답 전송
using BridgeCommandHandler = std::function<CString(const BridgeMessage&)>;

// Native(C++) ↔ WebView2(JS) JSON 브릿지 라우터
// 규약: webview2-bridge skill 참조
class BridgeDispatcher
{
public:
    BridgeDispatcher();

    // command 핸들러 등록 (target::action 키로 매핑)
    void RegisterHandler(
        const CString& strTarget,
        const CString& strAction,
        BridgeCommandHandler handler);

    // WebView2로부터 수신한 JSON 문자열 처리
    void DispatchMessage(const CString& strJson, ICoreWebView2* pWebView);

    // WebView2로 JSON 메시지 전송
    void PostMessageToWeb(const CString& strJson, ICoreWebView2* pWebView) const;

    // 편의 함수: event 전송
    void SendEvent(
        const CString& strEventName,
        const CString& strPayloadJson,
        ICoreWebView2* pWebView) const;

    // 편의 함수: success response 전송
    void SendSuccessResponse(
        const CString& strRequestId,
        const CString& strPayloadJson,
        ICoreWebView2* pWebView) const;

    // 편의 함수: error response 전송
    void SendErrorResponse(
        const CString& strRequestId,
        const CString& strErrorCode,
        const CString& strErrorMessage,
        ICoreWebView2* pWebView) const;

private:
    BridgeMessage ParseMessage(const CString& strJson) const;
    CString       ExtractJsonField(const CString& strJson, const CString& strKey) const;
    CString       MakeHandlerKey(const CString& strTarget, const CString& strAction) const;

    std::map<std::string, BridgeCommandHandler> m_mapHandlers;
};
