#include "pch.h"
#include "app/application/services/ApiCallService.h"
#include "Define.h"
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

BOOL ApiCallService::CallApi(const ApiCallAction& action, CString& strError)
{
    CString strDummy;
    return CallApi(action, strDummy, strError);
}

BOOL ApiCallService::CallApi(const ApiCallAction& action, CString& strResponseBody, CString& strError)
{
    if (action.m_strUrl.IsEmpty())
    {
        strError = L"URL이 비어 있습니다.";
        return FALSE;
    }

    CString strMethod = action.m_strMethod;
    if (strMethod.IsEmpty())
        strMethod = L"POST";

    int nTimeout = (action.m_nTimeoutMs > 0) ? action.m_nTimeoutMs : API_DEFAULT_TIMEOUT_MS;

    URL_COMPONENTS urlComp    = {};
    urlComp.dwStructSize      = sizeof(URL_COMPONENTS);

    wchar_t szScheme[32]      = {};
    wchar_t szHostName[512]   = {};
    wchar_t szUrlPath[2048]   = {};
    wchar_t szExtraInfo[1024] = {};

    urlComp.lpszScheme        = szScheme;
    urlComp.dwSchemeLength    = 32;
    urlComp.lpszHostName      = szHostName;
    urlComp.dwHostNameLength  = 512;
    urlComp.lpszUrlPath       = szUrlPath;
    urlComp.dwUrlPathLength   = 2048;
    urlComp.lpszExtraInfo     = szExtraInfo;
    urlComp.dwExtraInfoLength = 1024;

    if (!WinHttpCrackUrl(action.m_strUrl, action.m_strUrl.GetLength(), 0, &urlComp))
    {
        strError = L"URL 파싱 실패: " + action.m_strUrl;
        return FALSE;
    }

    HINTERNET hSession = WinHttpOpen(L"SageNexus/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession)
    {
        strError = L"WinHTTP 세션 생성 실패";
        return FALSE;
    }

    WinHttpSetTimeouts(hSession, nTimeout, nTimeout, nTimeout, nTimeout);

    BOOL bHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    HINTERNET hConnect = WinHttpConnect(hSession, szHostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        strError = L"서버 연결 실패: " + CString(szHostName);
        return FALSE;
    }

    CString strPath = BuildPath(szUrlPath, szExtraInfo);

    DWORD dwFlags    = bHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, strMethod,
        strPath, nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        strError = L"HTTP 요청 생성 실패";
        return FALSE;
    }

    ApplyHeaders(hRequest, action.m_strHeadersJson);

    std::string strBodyUtf8;
    if (!action.m_strBody.IsEmpty())
        strBodyUtf8 = WideToUtf8(action.m_strBody);

    BOOL bResult = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        strBodyUtf8.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)strBodyUtf8.c_str(),
        (DWORD)strBodyUtf8.size(),
        (DWORD)strBodyUtf8.size(), 0);

    if (bResult)
        bResult = WinHttpReceiveResponse(hRequest, nullptr);

    if (!bResult)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        strError = L"HTTP 요청 실패";
        return FALSE;
    }

    DWORD dwStatusCode = 0;
    DWORD dwStatusSize = sizeof(DWORD);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &dwStatusCode, &dwStatusSize, WINHTTP_NO_HEADER_INDEX);

    if (!ReadResponseBody(hRequest, strResponseBody))
        strResponseBody = L"";

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (dwStatusCode < 200 || dwStatusCode >= 300)
    {
        wchar_t szCode[16] = {};
        _itow_s((int)dwStatusCode, szCode, 16, 10);
        strError = L"API 요청 실패 (HTTP " + CString(szCode) + L")";
        return FALSE;
    }

    return TRUE;
}

CString ApiCallService::BuildPath(const wchar_t* szUrlPath, const wchar_t* szExtraInfo) const
{
    CString strPath = CString(szUrlPath) + CString(szExtraInfo);
    if (strPath.IsEmpty())
        strPath = L"/";
    return strPath;
}

void ApiCallService::ApplyHeaders(HINTERNET hRequest, const CString& strHeadersJson) const
{
    if (strHeadersJson.IsEmpty())
        return;

    std::string json = WideToUtf8(strHeadersJson);

    size_t nPos = 0;
    while (nPos < json.size())
    {
        size_t nKeyQuote = json.find('"', nPos);
        if (nKeyQuote == std::string::npos) break;

        size_t nKeyEnd = json.find('"', nKeyQuote + 1);
        if (nKeyEnd == std::string::npos) break;

        std::string strKey = json.substr(nKeyQuote + 1, nKeyEnd - nKeyQuote - 1);

        size_t nColon = json.find(':', nKeyEnd + 1);
        if (nColon == std::string::npos) break;

        size_t nValQuote = json.find('"', nColon + 1);
        if (nValQuote == std::string::npos) break;

        size_t nValEnd = nValQuote + 1;
        while (nValEnd < json.size())
        {
            if (json[nValEnd] == '\\') { nValEnd += 2; continue; }
            if (json[nValEnd] == '"') break;
            ++nValEnd;
        }

        if (nValEnd >= json.size()) break;

        std::string strVal = json.substr(nValQuote + 1, nValEnd - nValQuote - 1);

        CString strHeader = Utf8ToWide(strKey) + L": " + Utf8ToWide(strVal);
        WinHttpAddRequestHeaders(hRequest, strHeader,
            (DWORD)strHeader.GetLength(), WINHTTP_ADDREQ_FLAG_ADD);

        nPos = nValEnd + 1;
    }
}

BOOL ApiCallService::ReadResponseBody(HINTERNET hRequest, CString& strBody) const
{
    std::string strRaw;
    DWORD dwSize = 0;

    do
    {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;
        if (dwSize == 0)
            break;

        std::string buffer(dwSize, '\0');
        DWORD dwRead = 0;
        if (!WinHttpReadData(hRequest, &buffer[0], dwSize, &dwRead))
            break;

        strRaw.append(buffer, 0, dwRead);
    }
    while (dwSize > 0);

    if (strRaw.empty())
        return FALSE;

    strBody = Utf8ToWide(strRaw);
    return TRUE;
}
