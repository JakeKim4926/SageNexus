#include "pch.h"
#include "BridgeDispatcher.h"
#include "App/SageApp.h"

BridgeDispatcher::BridgeDispatcher()
{
}

void BridgeDispatcher::RegisterHandler(
    const CString& strTarget,
    const CString& strAction,
    BridgeCommandHandler handler)
{
    std::string key = WideToUtf8(MakeHandlerKey(strTarget, strAction));
    m_mapHandlers[key] = handler;
}

void BridgeDispatcher::DispatchMessage(const CString& strJson, ICoreWebView2* pWebView)
{
    BridgeMessage msg = ParseMessage(strJson);

    if (msg.m_eType == BridgeMessageType::Unknown)
    {
        sageMgr.GetLogger().LogWarning(L"BridgeDispatcher: Unknown message type received");
        return;
    }

    if (msg.m_eType != BridgeMessageType::Command)
        return; // response/event는 Web→Native 방향으로 오지 않음

    CString strLogMsg;
    strLogMsg.Format(L"Bridge command: %s::%s (reqId=%s)",
        (LPCWSTR)msg.m_strTarget, (LPCWSTR)msg.m_strAction, (LPCWSTR)msg.m_strRequestId);
    sageMgr.GetLogger().LogInfo(strLogMsg);

    std::string key = WideToUtf8(MakeHandlerKey(msg.m_strTarget, msg.m_strAction));
    std::map<std::string, BridgeCommandHandler>::iterator it = m_mapHandlers.find(key);

    if (it == m_mapHandlers.end())
    {
        CString strWarn;
        strWarn.Format(L"BridgeDispatcher: No handler for %s::%s",
            (LPCWSTR)msg.m_strTarget, (LPCWSTR)msg.m_strAction);
        sageMgr.GetLogger().LogWarning(strWarn);
        SendErrorResponse(msg.m_strRequestId, L"NOT_FOUND",
            L"No handler registered for this command", pWebView);
        return;
    }

    CString strResult = it->second(msg);

    // 핸들러가 비어있는 응답을 반환하면 기본 success 전송
    if (strResult.IsEmpty())
        SendSuccessResponse(msg.m_strRequestId, L"{}", pWebView);
    else
        PostMessageToWeb(strResult, pWebView);
}

void BridgeDispatcher::PostMessageToWeb(const CString& strJson, ICoreWebView2* pWebView) const
{
    if (!pWebView)
        return;

    pWebView->PostWebMessageAsString(strJson);
}

void BridgeDispatcher::SendEvent(
    const CString& strEventName,
    const CString& strPayloadJson,
    ICoreWebView2* pWebView) const
{
    CString strJson;
    strJson.Format(
        L"{\"type\":\"event\",\"name\":\"%s\",\"payload\":%s}",
        (LPCWSTR)strEventName,
        strPayloadJson.IsEmpty() ? L"{}" : (LPCWSTR)strPayloadJson);

    PostMessageToWeb(strJson, pWebView);
}

void BridgeDispatcher::SendSuccessResponse(
    const CString& strRequestId,
    const CString& strPayloadJson,
    ICoreWebView2* pWebView) const
{
    CString strJson;
    strJson.Format(
        L"{\"type\":\"response\",\"requestId\":\"%s\",\"success\":true,\"payload\":%s}",
        (LPCWSTR)strRequestId,
        strPayloadJson.IsEmpty() ? L"{}" : (LPCWSTR)strPayloadJson);

    PostMessageToWeb(strJson, pWebView);
}

void BridgeDispatcher::SendErrorResponse(
    const CString& strRequestId,
    const CString& strErrorCode,
    const CString& strErrorMessage,
    ICoreWebView2* pWebView) const
{
    CString strJson;
    strJson.Format(
        L"{\"type\":\"response\",\"requestId\":\"%s\",\"success\":false,"
        L"\"error\":{\"code\":\"%s\",\"message\":\"%s\"}}",
        (LPCWSTR)strRequestId,
        (LPCWSTR)strErrorCode,
        (LPCWSTR)strErrorMessage);

    PostMessageToWeb(strJson, pWebView);
}

BridgeMessage BridgeDispatcher::ParseMessage(const CString& strJson) const
{
    BridgeMessage msg;

    CString strType = ExtractJsonField(strJson, L"type");
    if (strType == L"command")
        msg.m_eType = BridgeMessageType::Command;
    else if (strType == L"response")
        msg.m_eType = BridgeMessageType::Response;
    else if (strType == L"event")
        msg.m_eType = BridgeMessageType::Event;
    else
        return msg;

    msg.m_strRequestId   = ExtractJsonField(strJson, L"requestId");
    msg.m_strTarget      = ExtractJsonField(strJson, L"target");
    msg.m_strAction      = ExtractJsonField(strJson, L"action");
    msg.m_strName        = ExtractJsonField(strJson, L"name");

    // payload 필드: 문자열 또는 오브젝트 형태 그대로 보존
    int nPayloadStart = strJson.Find(L"\"payload\"");
    if (nPayloadStart >= 0)
    {
        int nColon = strJson.Find(L':', nPayloadStart);
        if (nColon >= 0)
            msg.m_strPayloadJson = strJson.Mid(nColon + 1).TrimLeft();
    }

    return msg;
}

CString BridgeDispatcher::ExtractJsonField(const CString& strJson, const CString& strKey) const
{
    // 단순 문자열 값만 추출: "key": "value"
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0)
        return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0)
        return L"";

    // 값 시작 위치 탐색 (공백 건너뜀)
    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    if (nValueStart >= strJson.GetLength())
        return L"";

    if (strJson[nValueStart] != L'"')
        return L""; // 숫자/bool/오브젝트는 별도 처리 필요 시 확장

    int nValueEnd = strJson.Find(L'"', nValueStart + 1);
    if (nValueEnd < 0)
        return L"";

    return strJson.Mid(nValueStart + 1, nValueEnd - nValueStart - 1);
}

CString BridgeDispatcher::MakeHandlerKey(const CString& strTarget, const CString& strAction) const
{
    return strTarget + L"::" + strAction;
}
