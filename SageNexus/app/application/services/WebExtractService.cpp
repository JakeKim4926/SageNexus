#include "pch.h"
#include "app/application/services/WebExtractService.h"
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

BOOL WebExtractService::FetchAndExtract(const CString& strUrl, const CString& strSelector, DataTable& outTable, CString& strError)
{
    std::string strHtml;
    if (!FetchHtml(strUrl, strHtml, strError))
        return FALSE;

    return ExtractTable(strHtml, strSelector, outTable, strError);
}

BOOL WebExtractService::FetchHtml(const CString& strUrl, std::string& outHtml, CString& strError)
{
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

    if (!WinHttpCrackUrl(strUrl, strUrl.GetLength(), 0, &urlComp))
    {
        strError = L"URL 파싱 실패: " + strUrl;
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

    WinHttpSetTimeouts(hSession, 10000, 10000, 15000, 15000);

    BOOL bHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    HINTERNET hConnect = WinHttpConnect(hSession, szHostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        strError = L"서버 연결 실패: " + CString(szHostName);
        return FALSE;
    }

    CString strPath = CString(szUrlPath) + CString(szExtraInfo);
    if (strPath.IsEmpty())
        strPath = L"/";

    DWORD dwFlags    = bHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", strPath,
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        strError = L"HTTP 요청 생성 실패";
        return FALSE;
    }

    BOOL bResult = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

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

    outHtml.clear();
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

        outHtml.append(buffer, 0, dwRead);
    }
    while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (outHtml.empty())
    {
        strError = L"응답 본문이 비어 있습니다.";
        return FALSE;
    }

    return TRUE;
}

BOOL WebExtractService::ExtractTable(const std::string& strHtml, const CString& strSelector, DataTable& outTable, CString& strError)
{
    std::string strTableHtml = FindTableHtml(strHtml, strSelector);
    if (strTableHtml.empty())
    {
        strError = L"일치하는 테이블을 찾을 수 없습니다: " + (strSelector.IsEmpty() ? CString(L"table") : strSelector);
        return FALSE;
    }

    std::vector<std::string> rows = ExtractRowsFromTable(strTableHtml);
    if (rows.empty())
    {
        strError = L"테이블에 행이 없습니다.";
        return FALSE;
    }

    outTable.Clear();
    outTable.SetSourceName(strSelector.IsEmpty() ? L"web" : strSelector);

    std::vector<std::string> headers = ExtractCellsFromRow(rows[0], "th");
    int nDataStart = 1;

    if (headers.empty())
    {
        headers = ExtractCellsFromRow(rows[0], "td");
        nDataStart = 1;
    }

    for (int i = 0; i < (int)headers.size(); ++i)
    {
        CString strName = Utf8ToWide(headers[i]);
        if (strName.IsEmpty())
            strName.Format(L"Col%d", i + 1);

        DataColumn col;
        col.m_strInternalName  = strName;
        col.m_strSourceName    = strName;
        col.m_strDisplayNameKo = strName;
        col.m_strDisplayNameEn = strName;
        col.m_strDataType      = L"string";
        outTable.AddColumn(col);
    }

    int nColCount = outTable.GetColumnCount();
    if (nColCount == 0)
    {
        strError = L"테이블에서 컬럼을 추출할 수 없습니다.";
        return FALSE;
    }

    for (int i = nDataStart; i < (int)rows.size(); ++i)
    {
        std::vector<std::string> cells = ExtractCellsFromRow(rows[i], "td");
        if (cells.empty())
            continue;

        DataRow row;
        for (int j = 0; j < nColCount; ++j)
        {
            if (j < (int)cells.size())
                row.m_arrCells.push_back(Utf8ToWide(cells[j]));
            else
                row.m_arrCells.push_back(L"");
        }
        outTable.AddRow(row);
    }

    return TRUE;
}

std::string WebExtractService::FindTableHtml(const std::string& strHtml, const CString& strSelector)
{
    std::string selectorUtf8  = WideToUtf8(strSelector);
    std::string selectorLower = ToLower(selectorUtf8);
    std::string htmlLower     = ToLower(strHtml);

    std::string strFilter;
    if (selectorLower.empty() || selectorLower == "table")
        strFilter = "";
    else if (selectorLower.size() > 6 && selectorLower.substr(0, 6) == "table#")
        strFilter = selectorLower.substr(5);
    else if (selectorLower.size() > 6 && selectorLower.substr(0, 6) == "table.")
        strFilter = selectorLower.substr(5);
    else
        strFilter = selectorLower;

    size_t nSearch = 0;
    while (true)
    {
        size_t nTableStart = htmlLower.find("<table", nSearch);
        if (nTableStart == std::string::npos)
            return "";

        size_t nTagEnd = htmlLower.find('>', nTableStart);
        if (nTagEnd == std::string::npos)
            return "";

        std::string openTag = htmlLower.substr(nTableStart, nTagEnd - nTableStart + 1);
        bool bMatch = false;

        if (strFilter.empty())
        {
            bMatch = true;
        }
        else if (strFilter[0] == '#')
        {
            std::string idValue   = strFilter.substr(1);
            std::string pattern1  = "id=\"" + idValue + "\"";
            std::string pattern2  = "id='" + idValue + "'";
            bMatch = (openTag.find(pattern1) != std::string::npos ||
                      openTag.find(pattern2) != std::string::npos);
        }
        else if (strFilter[0] == '.')
        {
            std::string className = strFilter.substr(1);
            size_t nClass = openTag.find("class=");
            if (nClass != std::string::npos)
            {
                size_t nQuote = openTag.find_first_of("\"'", nClass + 6);
                if (nQuote != std::string::npos)
                {
                    char q = openTag[nQuote];
                    size_t nQuoteEnd = openTag.find(q, nQuote + 1);
                    if (nQuoteEnd != std::string::npos)
                    {
                        std::string classVal = openTag.substr(nQuote + 1, nQuoteEnd - nQuote - 1);
                        size_t nFound = classVal.find(className);
                        while (nFound != std::string::npos)
                        {
                            bool bBefore = (nFound == 0 || classVal[nFound - 1] == ' ');
                            bool bAfter  = (nFound + className.size() == classVal.size() || classVal[nFound + className.size()] == ' ');
                            if (bBefore && bAfter) { bMatch = true; break; }
                            nFound = classVal.find(className, nFound + 1);
                        }
                    }
                }
            }
        }

        if (!bMatch)
        {
            nSearch = nTableStart + 1;
            continue;
        }

        int nDepth    = 1;
        size_t nPos   = nTagEnd + 1;
        while (nDepth > 0 && nPos < htmlLower.size())
        {
            size_t nOpen  = htmlLower.find("<table", nPos);
            size_t nClose = htmlLower.find("</table", nPos);

            if (nClose == std::string::npos)
                return "";

            if (nOpen != std::string::npos && nOpen < nClose)
            {
                ++nDepth;
                nPos = nOpen + 1;
            }
            else
            {
                --nDepth;
                if (nDepth == 0)
                {
                    size_t nTableEnd = htmlLower.find('>', nClose);
                    if (nTableEnd == std::string::npos)
                        return "";
                    return strHtml.substr(nTableStart, nTableEnd - nTableStart + 1);
                }
                nPos = nClose + 1;
            }
        }

        return "";
    }
}

std::vector<std::string> WebExtractService::ExtractRowsFromTable(const std::string& strTableHtml)
{
    std::vector<std::string> rows;
    std::string lower = ToLower(strTableHtml);

    size_t nPos = 0;
    while (true)
    {
        size_t nStart = lower.find("<tr", nPos);
        if (nStart == std::string::npos)
            break;

        char cNext = (nStart + 3 < lower.size()) ? lower[nStart + 3] : '\0';
        if (cNext != '>' && cNext != ' ' && cNext != '\t' && cNext != '\r' && cNext != '\n')
        {
            nPos = nStart + 1;
            continue;
        }

        size_t nTagEnd = lower.find('>', nStart);
        if (nTagEnd == std::string::npos)
            break;

        size_t nClose = lower.find("</tr>", nTagEnd);
        if (nClose == std::string::npos)
            break;

        rows.push_back(strTableHtml.substr(nStart, nClose + 5 - nStart));
        nPos = nClose + 5;
    }

    return rows;
}

std::vector<std::string> WebExtractService::ExtractCellsFromRow(const std::string& strRowHtml, const std::string& strCellTag)
{
    std::vector<std::string> cells;
    std::string lower    = ToLower(strRowHtml);
    std::string openTag  = "<" + strCellTag;
    std::string closeTag = "</" + strCellTag + ">";

    size_t nPos = 0;
    while (true)
    {
        size_t nStart = lower.find(openTag, nPos);
        if (nStart == std::string::npos)
            break;

        char cNext = (nStart + openTag.size() < lower.size()) ? lower[nStart + openTag.size()] : '\0';
        if (cNext != '>' && cNext != ' ' && cNext != '\t' && cNext != '\r' && cNext != '\n')
        {
            nPos = nStart + 1;
            continue;
        }

        size_t nContentStart = lower.find('>', nStart);
        if (nContentStart == std::string::npos)
            break;
        ++nContentStart;

        size_t nClose = lower.find(closeTag, nContentStart);
        if (nClose == std::string::npos)
            break;

        std::string cellContent = strRowHtml.substr(nContentStart, nClose - nContentStart);
        cells.push_back(StripHtmlTags(DecodeHtmlEntities(cellContent)));
        nPos = nClose + closeTag.size();
    }

    return cells;
}

std::string WebExtractService::StripHtmlTags(const std::string& str)
{
    std::string result;
    bool bInTag = false;

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '<')       bInTag = true;
        else if (str[i] == '>') bInTag = false;
        else if (!bInTag)        result += str[i];
    }

    size_t nStart = result.find_first_not_of(" \t\r\n");
    size_t nEnd   = result.find_last_not_of(" \t\r\n");
    if (nStart == std::string::npos)
        return "";

    return result.substr(nStart, nEnd - nStart + 1);
}

std::string WebExtractService::DecodeHtmlEntities(const std::string& str)
{
    std::string result;
    size_t i = 0;

    while (i < str.size())
    {
        if (str[i] == '&')
        {
            size_t nSemi = str.find(';', i + 1);
            if (nSemi != std::string::npos && nSemi - i <= 10)
            {
                std::string entity = str.substr(i + 1, nSemi - i - 1);

                if      (entity == "amp")  { result += '&';  i = nSemi + 1; continue; }
                else if (entity == "lt")   { result += '<';  i = nSemi + 1; continue; }
                else if (entity == "gt")   { result += '>';  i = nSemi + 1; continue; }
                else if (entity == "quot") { result += '"';  i = nSemi + 1; continue; }
                else if (entity == "apos") { result += '\''; i = nSemi + 1; continue; }
                else if (entity == "nbsp") { result += ' ';  i = nSemi + 1; continue; }
                else if (!entity.empty() && entity[0] == '#')
                {
                    int nCode = 0;
                    if (entity.size() > 1 && entity[1] == 'x')
                        nCode = (int)strtol(entity.c_str() + 2, nullptr, 16);
                    else
                        nCode = atoi(entity.c_str() + 1);

                    if (nCode > 0 && nCode < 128) { result += (char)nCode; i = nSemi + 1; continue; }
                }
            }
        }
        result += str[i++];
    }

    return result;
}

std::string WebExtractService::ToLower(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i)
    {
        if (result[i] >= 'A' && result[i] <= 'Z')
            result[i] = result[i] - 'A' + 'a';
    }
    return result;
}
