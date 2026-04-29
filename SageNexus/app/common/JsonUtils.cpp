#include "pch.h"
#include "app/common/JsonUtils.h"

std::string JsonUtils::WideToUtf8Text(const CString& strWide)
{
    int nLen = WideCharToMultiByte(CP_UTF8, 0, strWide, -1, nullptr, 0, nullptr, nullptr);
    if (nLen <= 1)
        return "";

    std::string strUtf8(nLen - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strWide, -1, &strUtf8[0], nLen, nullptr, nullptr);
    return strUtf8;
}

CString JsonUtils::Utf8ToWideText(const std::string& strUtf8)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
    if (nLen <= 1)
        return L"";

    CString strWide;
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.GetBuffer(nLen), nLen);
    strWide.ReleaseBuffer();
    return strWide;
}

CString JsonUtils::ExtractString(const CString& strJson, const CString& strKey)
{
    std::string json = WideToUtf8Text(strJson);
    std::string key = WideToUtf8Text(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos)
        return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return L"";

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ')
        ++nStart;
    if (nStart >= json.size() || json[nStart] != '"')
        return L"";

    size_t nEnd = nStart + 1;
    while (nEnd < json.size())
    {
        if (json[nEnd] == '\\')
        {
            nEnd += 2;
            continue;
        }
        if (json[nEnd] == '"')
            break;
        ++nEnd;
    }
    if (nEnd >= json.size())
        return L"";

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
            case '"':  unescaped += '"'; break;
            case '\\': unescaped += '\\'; break;
            case '/':  unescaped += '/'; break;
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
    return Utf8ToWideText(unescaped);
}

BOOL JsonUtils::ExtractBool(const CString& strJson, const CString& strKey)
{
    std::string json = WideToUtf8Text(strJson);
    std::string key = WideToUtf8Text(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos)
        return FALSE;

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return FALSE;

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ')
        ++nStart;

    return (json.compare(nStart, 4, "true") == 0) ? TRUE : FALSE;
}

int JsonUtils::ExtractInt(const CString& strJson, const CString& strKey, int nDefaultValue)
{
    std::string json = WideToUtf8Text(strJson);
    std::string key = WideToUtf8Text(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos)
        return nDefaultValue;

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return nDefaultValue;

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ')
        ++nStart;

    int nSign = 1;
    if (nStart < json.size() && json[nStart] == '-')
    {
        nSign = -1;
        ++nStart;
    }

    int nValue = 0;
    BOOL bHasDigit = FALSE;
    while (nStart < json.size() && json[nStart] >= '0' && json[nStart] <= '9')
    {
        nValue = (nValue * 10) + (json[nStart] - '0');
        bHasDigit = TRUE;
        ++nStart;
    }

    return bHasDigit ? nValue * nSign : nDefaultValue;
}

CString JsonUtils::UnescapeText(const CString& strValue)
{
    CString strResult;
    for (int i = 0; i < strValue.GetLength(); ++i)
    {
        if (strValue[i] == L'\\' && i + 1 < strValue.GetLength())
        {
            ++i;
            switch (strValue[i])
            {
            case L'"':  strResult += L'"'; break;
            case L'\\': strResult += L'\\'; break;
            case L'/':  strResult += L'/'; break;
            case L'n':  strResult += L'\n'; break;
            case L'r':  strResult += L'\r'; break;
            case L't':  strResult += L'\t'; break;
            default:    strResult += strValue[i]; break;
            }
        }
        else
        {
            strResult += strValue[i];
        }
    }
    return strResult;
}

CString JsonUtils::EscapeString(const CString& strValue)
{
    CString strResult;
    for (int i = 0; i < strValue.GetLength(); ++i)
    {
        wchar_t ch = strValue[i];
        switch (ch)
        {
        case L'"':  strResult += L"\\\""; break;
        case L'\\': strResult += L"\\\\"; break;
        case L'\n': strResult += L"\\n"; break;
        case L'\r': strResult += L"\\r"; break;
        case L'\t': strResult += L"\\t"; break;
        default:    strResult += ch; break;
        }
    }
    return strResult;
}

std::string JsonUtils::EscapeText(const CString& strValue)
{
    CString strEscaped = EscapeString(strValue);
    return WideToUtf8Text(strEscaped);
}

BOOL JsonUtils::HasArrayKey(const std::string& strJson, const std::string& strKey)
{
    std::string strToken = "\"" + strKey + "\"";
    size_t nKeyPos = strJson.find(strToken);
    if (nKeyPos == std::string::npos)
        return FALSE;

    size_t nColon = strJson.find(':', nKeyPos + strToken.size());
    if (nColon == std::string::npos)
        return FALSE;

    size_t nArrayStart = strJson.find('[', nColon + 1);
    return nArrayStart == std::string::npos ? FALSE : TRUE;
}

std::string JsonUtils::ExtractArrayText(const std::string& strJson, const std::string& strKey)
{
    std::string strToken = "\"" + strKey + "\"";
    size_t nKeyPos = strJson.find(strToken);
    if (nKeyPos == std::string::npos)
        return "";

    size_t nColon = strJson.find(':', nKeyPos + strToken.size());
    if (nColon == std::string::npos)
        return "";

    size_t nArrayStart = strJson.find('[', nColon + 1);
    if (nArrayStart == std::string::npos)
        return "";

    size_t nArrayEnd = FindArrayEnd(strJson, nArrayStart);
    if (nArrayEnd == std::string::npos || nArrayEnd <= nArrayStart)
        return "";

    return strJson.substr(nArrayStart + 1, nArrayEnd - nArrayStart - 1);
}

void JsonUtils::ExtractObjects(const std::string& strArrayText, std::vector<std::string>& outObjects)
{
    BOOL bInString = FALSE;
    int nDepth = 0;
    size_t nObjectStart = std::string::npos;

    for (size_t i = 0; i < strArrayText.size(); ++i)
    {
        if (strArrayText[i] == '\\' && bInString)
        {
            ++i;
            continue;
        }

        if (strArrayText[i] == '"')
        {
            bInString = !bInString;
            continue;
        }

        if (bInString)
            continue;

        if (strArrayText[i] == '{')
        {
            if (nDepth == 0)
                nObjectStart = i;
            ++nDepth;
        }
        else if (strArrayText[i] == '}')
        {
            --nDepth;
            if (nDepth == 0 && nObjectStart != std::string::npos)
            {
                outObjects.push_back(strArrayText.substr(nObjectStart, i - nObjectStart + 1));
                nObjectStart = std::string::npos;
            }
        }
    }
}

CString JsonUtils::ExtractObjectString(const std::string& strObject, const CString& strKey)
{
    return ExtractString(Utf8ToWideText(strObject), strKey);
}

BOOL JsonUtils::ExtractObjectBool(const std::string& strObject, const CString& strKey)
{
    return ExtractBool(Utf8ToWideText(strObject), strKey);
}

size_t JsonUtils::FindArrayEnd(const std::string& strJson, size_t nArrayStart)
{
    int nDepth = 0;
    BOOL bInString = FALSE;
    for (size_t i = nArrayStart; i < strJson.size(); ++i)
    {
        if (strJson[i] == '\\' && bInString)
        {
            ++i;
            continue;
        }

        if (strJson[i] == '"')
        {
            bInString = !bInString;
            continue;
        }

        if (bInString)
            continue;

        if (strJson[i] == '[')
            ++nDepth;
        else if (strJson[i] == ']')
        {
            --nDepth;
            if (nDepth == 0)
                return i;
        }
    }

    return std::string::npos;
}

std::string WideToUtf8(const CString& strWide)
{
    return JsonUtils::WideToUtf8Text(strWide);
}

CString Utf8ToWide(const std::string& strUtf8)
{
    return JsonUtils::Utf8ToWideText(strUtf8);
}

CString JsonExtractString(const CString& strJson, const CString& strKey)
{
    return JsonUtils::ExtractString(strJson, strKey);
}

BOOL JsonExtractBool(const CString& strJson, const CString& strKey)
{
    return JsonUtils::ExtractBool(strJson, strKey);
}

CString UnescapeJsonString(const CString& str)
{
    return JsonUtils::UnescapeText(str);
}

CString JsonEscapeString(const CString& str)
{
    return JsonUtils::EscapeString(str);
}
