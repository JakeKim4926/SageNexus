#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/SageApp.h"
#include "Define.h"

BridgeDispatcher::BridgeDispatcher()
    : m_hMainWnd(nullptr)
    , m_pCachedWebView(nullptr)
{
}

void BridgeDispatcher::SetMainHwnd(HWND hMainWnd)
{
    m_hMainWnd = hMainWnd;
}

void BridgeDispatcher::SetWebView(ICoreWebView2* pWebView)
{
    m_pCachedWebView = pWebView;
}

void BridgeDispatcher::RegisterHandler(
    const CString& strTarget,
    const CString& strAction,
    BridgeCommandHandler handler)
{
    std::string key = WideToUtf8(MakeHandlerKey(strTarget, strAction));
    m_mapHandlers[key] = handler;
}

void BridgeDispatcher::RegisterDeferredHandler(
    const CString& strTarget,
    const CString& strAction,
    BridgeCommandHandler handler)
{
    std::string key = WideToUtf8(MakeHandlerKey(strTarget, strAction));
    m_mapHandlers[key]        = handler;
    m_deferredHandlerKeys.insert(key);
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
        return;

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

    if (m_deferredHandlerKeys.count(key) > 0)
    {
        // 파일 다이얼로그 등 중첩 메시지 루프가 필요한 핸들러:
        // WebView2 이벤트 콜백 밖에서 실행하도록 WndProc에 위임한다
        DeferredCmd* pCmd   = new DeferredCmd();
        pCmd->m_msg         = msg;
        pCmd->m_handler     = it->second;
        PostMessageW(m_hMainWnd, WM_BRIDGE_DEFERRED_CMD, 0, reinterpret_cast<LPARAM>(pCmd));
        return;
    }

    CString strResult = it->second(msg);

    if (strResult.IsEmpty())
        SendSuccessResponse(msg.m_strRequestId, L"{}", pWebView);
    else
        PostMessageToWeb(strResult, pWebView);
}

void BridgeDispatcher::PostResponse(const CString& strJson) const
{
    PostMessageToWeb(strJson, m_pCachedWebView);
}

void BridgeDispatcher::PostMessageToWeb(const CString& strJson, ICoreWebView2* pWebView) const
{
    if (!pWebView)
        return;

    // virtual host(https://app.sagenexus) 하에서 PostWebMessageAsString 경로가
    // JS message 리스너로 전달되지 않아 ExecuteScript로 CustomEvent를 직접 dispatch한다.
    CString strEscaped = strJson;
    strEscaped.Replace(L"\\", L"\\\\");
    strEscaped.Replace(L"\"", L"\\\"");
    strEscaped.Replace(L"\r", L"");
    strEscaped.Replace(L"\n", L"\\n");

    CString strScript;
    strScript.Format(
        L"(function(){"
        L"try{"
        L"var m=JSON.parse(\"%s\");"
        L"if(m&&m.type==='event'&&m.name){"
        L"window.dispatchEvent(new CustomEvent('bridge:'+m.name,{detail:m.payload||{}}));"
        L"}"
        L"if(window.__bridgeReceive){window.__bridgeReceive(m);}"
        L"var fs=document.getElementsByTagName('iframe');"
        L"for(var i=0;i<fs.length;i++){"
        L"try{var cw=fs[i].contentWindow;if(cw&&cw.__bridgeReceive){cw.__bridgeReceive(m);}}catch(e){}"
        L"}"
        L"}catch(e){}"
        L"})();",
        (LPCWSTR)strEscaped);

    HRESULT hr = pWebView->ExecuteScript(strScript, nullptr);
    if (FAILED(hr))
    {
        CString strErr;
        strErr.Format(L"ExecuteScript failed hr=0x%08X", (unsigned int)hr);
        sageMgr.GetLogger().LogError(strErr);
    }
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
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0)
        return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0)
        return L"";

    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    if (nValueStart >= strJson.GetLength())
        return L"";

    if (strJson[nValueStart] != L'"')
        return L"";

    int nValueEnd = strJson.Find(L'"', nValueStart + 1);
    if (nValueEnd < 0)
        return L"";

    return strJson.Mid(nValueStart + 1, nValueEnd - nValueStart - 1);
}

CString BridgeDispatcher::MakeHandlerKey(const CString& strTarget, const CString& strAction) const
{
    return strTarget + L"::" + strAction;
}
