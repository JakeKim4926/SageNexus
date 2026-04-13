#include "pch.h"
#include "app/application/services/ApiConnectorService.h"
#include "app/application/services/ApiCallService.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <fstream>
#include <sstream>

ApiConnectorService::ApiConnectorService()
{
}

void ApiConnectorService::LoadFromFile()
{
    CString strPath = sageMgr.GetUserDataDir() + L"\\" + CONNECTORS_FILE_NAME;
    std::string strFilePath = WideToUtf8(strPath);

    std::ifstream file(strFilePath);
    if (!file.is_open())
        return;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string strContent = ss.str();

    m_arrConnectors.clear();

    size_t nListPos = strContent.find("\"connectors\"");
    if (nListPos == std::string::npos)
        return;

    size_t nArrayStart = strContent.find('[', nListPos);
    if (nArrayStart == std::string::npos)
        return;

    size_t nArrayEnd = strContent.rfind(']');
    if (nArrayEnd == std::string::npos || nArrayEnd <= nArrayStart)
        return;

    std::string strArray = strContent.substr(nArrayStart + 1, nArrayEnd - nArrayStart - 1);

    size_t nPos = 0;
    while (nPos < strArray.size())
    {
        size_t nObjStart = strArray.find('{', nPos);
        if (nObjStart == std::string::npos)
            break;

        int nDepth = 1;
        size_t nObjEnd = nObjStart + 1;
        while (nObjEnd < strArray.size() && nDepth > 0)
        {
            if (strArray[nObjEnd] == '{') ++nDepth;
            else if (strArray[nObjEnd] == '}') --nDepth;
            ++nObjEnd;
        }
        --nObjEnd;

        std::string strObj = strArray.substr(nObjStart + 1, nObjEnd - nObjStart - 1);

        ApiConnector conn;
        conn.m_strConnectorId  = ExtractStringField(strObj, "connectorId");
        conn.m_strName         = ExtractStringField(strObj, "name");
        conn.m_strBaseUrl      = ExtractStringField(strObj, "baseUrl");
        conn.m_strHeadersJson  = ExtractStringField(strObj, "headersJson");
        conn.m_strAuthType     = ExtractStringField(strObj, "authType");
        conn.m_strAuthValue    = ExtractStringField(strObj, "authValue");
        conn.m_bEnabled        = ExtractBoolField(strObj, "enabled");
        conn.m_strCreatedAt    = ExtractStringField(strObj, "createdAt");

        if (!conn.m_strConnectorId.IsEmpty() && !conn.m_strName.IsEmpty())
            m_arrConnectors.push_back(conn);

        nPos = nObjEnd + 1;
    }
}

void ApiConnectorService::SaveToFile() const
{
    CString strPath = sageMgr.GetUserDataDir() + L"\\" + CONNECTORS_FILE_NAME;
    std::string strFilePath = WideToUtf8(strPath);

    std::ofstream file(strFilePath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return;

    file << "{\n  \"connectors\": [\n";
    for (int i = 0; i < (int)m_arrConnectors.size(); ++i)
    {
        const ApiConnector& conn = m_arrConnectors[i];
        file << "    {\n";
        file << "      \"connectorId\": \""  << WideToUtf8(conn.m_strConnectorId)  << "\",\n";
        file << "      \"name\": \""         << WideToUtf8(conn.m_strName)         << "\",\n";
        file << "      \"baseUrl\": \""      << WideToUtf8(conn.m_strBaseUrl)      << "\",\n";
        file << "      \"headersJson\": \""  << WideToUtf8(conn.m_strHeadersJson)  << "\",\n";
        file << "      \"authType\": \""     << WideToUtf8(conn.m_strAuthType)     << "\",\n";
        file << "      \"authValue\": \""    << WideToUtf8(conn.m_strAuthValue)    << "\",\n";
        file << "      \"enabled\": "        << (conn.m_bEnabled ? "true" : "false") << ",\n";
        file << "      \"createdAt\": \""    << WideToUtf8(conn.m_strCreatedAt)    << "\"\n";
        file << "    }";
        if (i < (int)m_arrConnectors.size() - 1)
            file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
}

void ApiConnectorService::AddConnector(const CString& strName, const CString& strBaseUrl,
                                        const CString& strHeadersJson, const CString& strAuthType,
                                        const CString& strAuthValue, CString& strIdOut)
{
    ApiConnector conn;
    conn.m_strConnectorId = GenerateConnectorId();
    conn.m_strName        = strName;
    conn.m_strBaseUrl     = strBaseUrl;
    conn.m_strHeadersJson = strHeadersJson;
    conn.m_strAuthType    = strAuthType.IsEmpty() ? CString(L"none") : strAuthType;
    conn.m_strAuthValue   = strAuthValue;
    conn.m_bEnabled       = TRUE;

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    conn.m_strCreatedAt.Format(L"%04d-%02d-%02d %02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

    m_arrConnectors.push_back(conn);
    strIdOut = conn.m_strConnectorId;
    SaveToFile();
}

void ApiConnectorService::RemoveConnector(const CString& strId)
{
    for (int i = 0; i < (int)m_arrConnectors.size(); ++i)
    {
        if (m_arrConnectors[i].m_strConnectorId == strId)
        {
            m_arrConnectors.erase(m_arrConnectors.begin() + i);
            break;
        }
    }
    SaveToFile();
}

void ApiConnectorService::UpdateConnector(const CString& strId, const CString& strName,
                                           const CString& strBaseUrl, const CString& strHeadersJson,
                                           const CString& strAuthType, const CString& strAuthValue)
{
    for (ApiConnector& conn : m_arrConnectors)
    {
        if (conn.m_strConnectorId == strId)
        {
            conn.m_strName        = strName;
            conn.m_strBaseUrl     = strBaseUrl;
            conn.m_strHeadersJson = strHeadersJson;
            conn.m_strAuthType    = strAuthType.IsEmpty() ? CString(L"none") : strAuthType;
            conn.m_strAuthValue   = strAuthValue;
            break;
        }
    }
    SaveToFile();
}

void ApiConnectorService::GetConnectors(std::vector<ApiConnector>& arrConnectors) const
{
    arrConnectors = m_arrConnectors;
}

BOOL ApiConnectorService::GetConnector(const CString& strId, ApiConnector& outConnector) const
{
    for (const ApiConnector& conn : m_arrConnectors)
    {
        if (conn.m_strConnectorId == strId)
        {
            outConnector = conn;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ApiConnectorService::TestConnector(const CString& strId, CString& strError)
{
    ApiConnector conn;
    if (!GetConnector(strId, conn))
    {
        strError = L"커넥터를 찾을 수 없습니다.";
        return FALSE;
    }

    ApiCallAction action;
    action.m_strUrl         = conn.m_strBaseUrl;
    action.m_strMethod      = L"GET";
    action.m_strHeadersJson = BuildMergedHeaders(conn);
    action.m_nTimeoutMs     = API_DEFAULT_TIMEOUT_MS;

    ApiCallService svc;
    return svc.CallApi(action, strError);
}

BOOL ApiConnectorService::BuildAction(const CString& strId, const CString& strUrlSuffix,
                                       const CString& strMethod, const CString& strBody,
                                       ApiCallAction& outAction, CString& strError) const
{
    ApiConnector conn;
    if (!GetConnector(strId, conn))
    {
        strError = L"커넥터를 찾을 수 없습니다: " + strId;
        return FALSE;
    }

    // URL 조합: suffix가 http로 시작하면 단독 URL, '/'로 시작하면 baseUrl에 붙임, 없으면 baseUrl 사용
    CString strUrl;
    if (!strUrlSuffix.IsEmpty() && strUrlSuffix.Left(4) == L"http")
    {
        strUrl = strUrlSuffix;
    }
    else if (!strUrlSuffix.IsEmpty())
    {
        CString strBase = conn.m_strBaseUrl;
        if (strBase.Right(1) == L"/")
            strBase = strBase.Left(strBase.GetLength() - 1);
        CString strSuffix = strUrlSuffix;
        if (strSuffix.Left(1) != L"/")
            strSuffix = L"/" + strSuffix;
        strUrl = strBase + strSuffix;
    }
    else
    {
        strUrl = conn.m_strBaseUrl;
    }

    if (strUrl.IsEmpty())
    {
        strError = L"커넥터 URL이 비어 있습니다.";
        return FALSE;
    }

    outAction.m_strUrl         = strUrl;
    outAction.m_strMethod      = strMethod.IsEmpty() ? CString(L"POST") : strMethod;
    outAction.m_strHeadersJson = BuildMergedHeaders(conn);
    outAction.m_strBody        = strBody;
    outAction.m_nTimeoutMs     = API_DEFAULT_TIMEOUT_MS;

    return TRUE;
}

CString ApiConnectorService::GenerateConnectorId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"CONN-%04d%02d%02d-%02d%02d-%04d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wMilliseconds);
    return strId;
}

CString ApiConnectorService::BuildMergedHeaders(const ApiConnector& conn) const
{
    CString strBase = conn.m_strHeadersJson;

    if (conn.m_strAuthType == L"bearer" && !conn.m_strAuthValue.IsEmpty())
    {
        CString strPair;
        strPair.Format(L"\"Authorization\":\"Bearer %s\"",
                       (LPCWSTR)EscapeJsonValue(conn.m_strAuthValue));
        return InjectJsonPair(strBase, strPair);
    }
    else if (conn.m_strAuthType == L"apikey" && !conn.m_strAuthValue.IsEmpty())
    {
        CString strPair;
        strPair.Format(L"\"X-API-Key\":\"%s\"",
                       (LPCWSTR)EscapeJsonValue(conn.m_strAuthValue));
        return InjectJsonPair(strBase, strPair);
    }

    return strBase.IsEmpty() ? CString(L"{}") : strBase;
}

CString ApiConnectorService::InjectJsonPair(const CString& strJson, const CString& strPair) const
{
    CString strResult = strJson;
    strResult.Trim();
    if (strResult.IsEmpty() || strResult == L"{}")
        return L"{" + strPair + L"}";

    int nLast = strResult.ReverseFind(L'}');
    if (nLast < 0)
        return L"{" + strPair + L"}";

    CString strInner = strResult.Mid(1, nLast - 1);
    strInner.Trim();
    if (strInner.IsEmpty())
        return L"{" + strPair + L"}";

    return strResult.Left(nLast) + L"," + strPair + L"}";
}

CString ApiConnectorService::EscapeJsonValue(const CString& str) const
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

CString ApiConnectorService::ExtractStringField(const std::string& strObj, const std::string& strKey) const
{
    std::string token = "\"" + strKey + "\"";
    size_t nKeyPos = strObj.find(token);
    if (nKeyPos == std::string::npos)
        return L"";

    size_t nColon = strObj.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return L"";

    size_t nOpen = strObj.find('"', nColon + 1);
    if (nOpen == std::string::npos)
        return L"";

    size_t nClose = strObj.find('"', nOpen + 1);
    if (nClose == std::string::npos)
        return L"";

    return Utf8ToWide(strObj.substr(nOpen + 1, nClose - nOpen - 1));
}

BOOL ApiConnectorService::ExtractBoolField(const std::string& strObj, const std::string& strKey) const
{
    std::string token = "\"" + strKey + "\"";
    size_t nKeyPos = strObj.find(token);
    if (nKeyPos == std::string::npos)
        return FALSE;

    size_t nColon = strObj.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return FALSE;

    size_t nStart = nColon + 1;
    while (nStart < strObj.size() && strObj[nStart] == ' ')
        ++nStart;

    return (strObj.substr(nStart, 4) == "true") ? TRUE : FALSE;
}
