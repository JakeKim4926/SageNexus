#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <shlobj.h>
#include <atlbase.h>
#include <atlstr.h>
#include <wrl.h>
#include <wrl/event.h>

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include <mutex>
#include <map>
#include <set>
#include <cassert>

#include <WebView2.h>

// UTF-8 변환 유틸리티
inline std::string WideToUtf8(const CString& strWide)
{
    int nLen = WideCharToMultiByte(CP_UTF8, 0, strWide, -1, nullptr, 0, nullptr, nullptr);
    if (nLen <= 1)
        return "";

    std::string strUtf8(nLen - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strWide, -1, &strUtf8[0], nLen, nullptr, nullptr);
    return strUtf8;
}

inline CString Utf8ToWide(const std::string& strUtf8)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
    if (nLen <= 1)
        return L"";

    CString strWide;
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.GetBuffer(nLen), nLen);
    strWide.ReleaseBuffer();
    return strWide;
}

// ============================================================
// 공유 JSON 유틸리티
// 이스케이프 처리가 포함된 JSON 문자열 필드 추출 및 이스케이프
// BridgeHandler 전체에서 공통으로 사용한다.
// ============================================================

// JSON 오브젝트에서 strKey에 해당하는 문자열 값을 추출하고 언이스케이프하여 반환한다.
// 이스케이프된 따옴표(\\")를 올바르게 처리한다.
// 값이 없거나 문자열 타입이 아니면 빈 문자열을 반환한다.
inline CString JsonExtractString(const CString& strJson, const CString& strKey)
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return L"";

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ') ++nStart;
    if (nStart >= json.size() || json[nStart] != '"') return L"";

    // 이스케이프를 고려한 닫는 따옴표 탐색
    size_t nEnd = nStart + 1;
    while (nEnd < json.size())
    {
        if (json[nEnd] == '\\') { nEnd += 2; continue; }
        if (json[nEnd] == '"')  break;
        ++nEnd;
    }
    if (nEnd >= json.size()) return L"";

    // 이스케이프 해제
    std::string raw = json.substr(nStart + 1, nEnd - nStart - 1);
    std::string unescaped;
    unescaped.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i)
    {
        if (raw[i] == '\\' && i + 1 < raw.size())
        {
            ++i;
            switch (raw[i])
            {
            case '"':  unescaped += '"';  break;
            case '\\': unescaped += '\\'; break;
            case '/':  unescaped += '/';  break;
            case 'n':  unescaped += '\n'; break;
            case 'r':  unescaped += '\r'; break;
            case 't':  unescaped += '\t'; break;
            default:   unescaped += raw[i]; break;
            }
        }
        else
        {
            unescaped += raw[i];
        }
    }
    return Utf8ToWide(unescaped);
}

// JSON 오브젝트에서 bool 필드를 추출한다. true/false 리터럴만 인식한다.
inline BOOL JsonExtractBool(const CString& strJson, const CString& strKey)
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return FALSE;

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return FALSE;

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ') ++nStart;

    return (json.compare(nStart, 4, "true") == 0) ? TRUE : FALSE;
}

// JSON 이스케이프 시퀀스를 원래 문자로 복원한다.
inline CString UnescapeJsonString(const CString& str)
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        if (str[i] == L'\\' && i + 1 < str.GetLength())
        {
            ++i;
            switch (str[i])
            {
            case L'"':  strResult += L'"';  break;
            case L'\\': strResult += L'\\'; break;
            case L'/':  strResult += L'/';  break;
            case L'n':  strResult += L'\n'; break;
            case L'r':  strResult += L'\r'; break;
            case L't':  strResult += L'\t'; break;
            default:    strResult += str[i]; break;
            }
        }
        else
        {
            strResult += str[i];
        }
    }
    return strResult;
}

// JSON 특수문자를 이스케이프하여 문자열 값으로 안전하게 삽입할 수 있는 형태로 반환한다.
inline CString JsonEscapeString(const CString& str)
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        switch (ch)
        {
        case L'"':  strResult += L"\\\""; break;
        case L'\\': strResult += L"\\\\"; break;
        case L'\n': strResult += L"\\n";  break;
        case L'\r': strResult += L"\\r";  break;
        case L'\t': strResult += L"\\t";  break;
        default:    strResult += ch;      break;
        }
    }
    return strResult;
}
