#include "pch.h"
#include "JsonConfigStore.h"
#include "../Define.h"

JsonConfigStore::JsonConfigStore(const CString& strConfigDir)
    : m_strConfigDir(strConfigDir)
{
    // 기본값 설정
    m_mapEntries["interfaceLanguage"] = "ko";
    m_mapEntries["outputLanguage"]    = "ko";
    m_mapEntries["theme"]             = "dark";
    m_mapEntries["windowWidth"]       = "1280";
    m_mapEntries["windowHeight"]      = "800";
}

BOOL JsonConfigStore::Load()
{
    std::string strPath = WideToUtf8(BuildFilePath());
    std::ifstream file(strPath);
    if (!file.is_open())
        return FALSE; // 파일 없으면 기본값 유지

    std::stringstream ss;
    ss << file.rdbuf();
    ParseJsonText(ss.str());
    return TRUE;
}

BOOL JsonConfigStore::Save() const
{
    std::string strPath = WideToUtf8(BuildFilePath());
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return FALSE;

    file << BuildJsonText();
    return TRUE;
}

CString JsonConfigStore::GetString(const CString& strKey, const CString& strDefault) const
{
    std::string key = WideToUtf8(strKey);
    std::map<std::string, std::string>::const_iterator it = m_mapEntries.find(key);
    if (it == m_mapEntries.end())
        return strDefault;
    return Utf8ToWide(it->second);
}

int JsonConfigStore::GetInt(const CString& strKey, int nDefault) const
{
    CString strValue = GetString(strKey, L"");
    if (strValue.IsEmpty())
        return nDefault;
    return _wtoi(strValue);
}

BOOL JsonConfigStore::GetBool(const CString& strKey, BOOL bDefault) const
{
    CString strValue = GetString(strKey, L"");
    if (strValue.IsEmpty())
        return bDefault;
    return (strValue == L"true") ? TRUE : FALSE;
}

void JsonConfigStore::SetString(const CString& strKey, const CString& strValue)
{
    m_mapEntries[WideToUtf8(strKey)] = WideToUtf8(strValue);
}

void JsonConfigStore::SetInt(const CString& strKey, int nValue)
{
    m_mapEntries[WideToUtf8(strKey)] = std::to_string(nValue);
}

void JsonConfigStore::SetBool(const CString& strKey, BOOL bValue)
{
    m_mapEntries[WideToUtf8(strKey)] = bValue ? "true" : "false";
}

CString JsonConfigStore::BuildFilePath() const
{
    return m_strConfigDir + L"\\" + CONFIG_FILE_NAME;
}

std::string JsonConfigStore::BuildJsonText() const
{
    std::stringstream ss;
    ss << "{\n";

    BOOL bFirst = TRUE;
    for (std::map<std::string, std::string>::const_iterator it = m_mapEntries.begin();
         it != m_mapEntries.end(); ++it)
    {
        if (!bFirst)
            ss << ",\n";
        bFirst = FALSE;

        // 간단한 escape: 쌍따옴표만 처리
        std::string strValue = it->second;
        bool bIsNumber = !strValue.empty() &&
            (isdigit((unsigned char)strValue[0]) || strValue[0] == '-');
        bool bIsBool = (strValue == "true" || strValue == "false");

        ss << "  \"" << it->first << "\": ";
        if (bIsBool || bIsNumber)
            ss << strValue;
        else
            ss << "\"" << strValue << "\"";
    }
    ss << "\n}";
    return ss.str();
}

void JsonConfigStore::ParseJsonText(const std::string& strJson)
{
    // Phase 1: 단순 라인 기반 파서
    // "key": "value" 또는 "key": 42 또는 "key": true 형태 파싱
    std::istringstream ss(strJson);
    std::string strLine;

    while (std::getline(ss, strLine))
    {
        size_t nColon = strLine.find(':');
        if (nColon == std::string::npos)
            continue;

        std::string strKey = strLine.substr(0, nColon);
        std::string strVal = strLine.substr(nColon + 1);

        // 키에서 공백, 따옴표, { 제거
        std::string strCleanKey;
        for (char c : strKey)
        {
            if (c != ' ' && c != '"' && c != '{' && c != '\t')
                strCleanKey += c;
        }

        // 값에서 공백, 따옴표, }, , 제거
        std::string strCleanVal;
        for (char c : strVal)
        {
            if (c != ' ' && c != '"' && c != '}' && c != ',' && c != '\t')
                strCleanVal += c;
        }

        if (!strCleanKey.empty() && !strCleanVal.empty())
            m_mapEntries[strCleanKey] = strCleanVal;
    }
}
