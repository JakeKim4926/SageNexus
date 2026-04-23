#include "pch.h"
#include "SolutionProfile.h"

SolutionProfile::SolutionProfile()
{
    SetDefault();
}

void SolutionProfile::SetDefault()
{
    m_strProfileId                = L"default";
    m_strProfileName              = L"Default Profile";
    m_strDefaultInterfaceLanguage = L"ko";
    m_strDefaultOutputLanguage    = L"ko";
    m_menuVisibility              = MenuVisibility();
    m_arrPlugins.clear();
}

BOOL SolutionProfile::LoadFromFile(const CString& strPath, CString& strError)
{
    std::ifstream file(WideToUtf8(strPath), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        strError = L"프로필 파일을 열 수 없습니다: " + strPath;
        return FALSE;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return LoadFromJsonString(ss.str(), strError);
}

BOOL SolutionProfile::LoadFromResource(int nResourceId, CString& strError)
{
    HMODULE hMod = GetModuleHandleW(nullptr);
    HRSRC   hRes = FindResourceW(hMod, MAKEINTRESOURCEW(nResourceId), RT_RCDATA);
    if (!hRes)
    {
        strError = L"프로필 리소스를 찾을 수 없습니다.";
        return FALSE;
    }

    HGLOBAL hGlobal = LoadResource(hMod, hRes);
    if (!hGlobal)
    {
        strError = L"프로필 리소스 로드에 실패했습니다.";
        return FALSE;
    }

    const char* pData  = static_cast<const char*>(LockResource(hGlobal));
    DWORD       dwSize = SizeofResource(hMod, hRes);
    if (!pData || dwSize == 0)
    {
        strError = L"프로필 리소스가 비어있습니다.";
        return FALSE;
    }

    std::string strJson(pData, dwSize);
    return LoadFromJsonString(strJson, strError);
}

BOOL SolutionProfile::SaveToFile(const CString& strPath, CString& strError) const
{
    std::ofstream file(WideToUtf8(strPath), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.is_open())
    {
        strError = L"프로필 파일을 저장할 수 없습니다: " + strPath;
        return FALSE;
    }

    file << BuildJsonString();
    if (!file.good())
    {
        strError = L"프로필 파일 저장 중 오류가 발생했습니다: " + strPath;
        return FALSE;
    }

    return TRUE;
}

std::string SolutionProfile::BuildJsonString() const
{
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"profileId\": \"" << WideToUtf8(m_strProfileId) << "\",\n";
    ss << "  \"profileName\": \"" << WideToUtf8(m_strProfileName) << "\",\n";
    ss << "  \"defaultInterfaceLanguage\": \"" << WideToUtf8(m_strDefaultInterfaceLanguage) << "\",\n";
    ss << "  \"defaultOutputLanguage\": \"" << WideToUtf8(m_strDefaultOutputLanguage) << "\",\n";
    ss << "  \"showDataViewer\": " << (m_menuVisibility.m_bShowDataViewer ? "true" : "false") << ",\n";
    ss << "  \"showTransform\": " << (m_menuVisibility.m_bShowTransform ? "true" : "false") << ",\n";
    ss << "  \"showExport\": " << (m_menuVisibility.m_bShowExport ? "true" : "false") << ",\n";
    ss << "  \"showHistory\": " << (m_menuVisibility.m_bShowHistory ? "true" : "false") << ",\n";
    ss << "  \"showWorkflow\": " << (m_menuVisibility.m_bShowWorkflow ? "true" : "false") << ",\n";
    ss << "  \"showWebextract\": " << (m_menuVisibility.m_bShowWebextract ? "true" : "false") << ",\n";
    ss << "  \"showSettings\": " << (m_menuVisibility.m_bShowSettings ? "true" : "false");

    for (int i = 0; i < static_cast<int>(m_arrPlugins.size()); ++i)
    {
        ss << ",\n";
        ss << "  \"plugin_" << WideToUtf8(m_arrPlugins[i].m_strPluginId) << "\": "
           << (m_arrPlugins[i].m_bEnabled ? "true" : "false");
    }

    ss << "\n}";
    return ss.str();
}

BOOL SolutionProfile::LoadFromJsonString(const std::string& strJson, CString& strError)
{
    std::map<std::string, std::string> entries;
    ParseProfileJson(strJson, entries);

    if (entries.find("profileId") == entries.end())
    {
        strError = L"유효하지 않은 프로필 형식입니다.";
        return FALSE;
    }

    SetDefault();

    m_strProfileId                = Utf8ToWide(entries["profileId"]);
    m_strProfileName              = Utf8ToWide(entries["profileName"]);
    m_strDefaultInterfaceLanguage = Utf8ToWide(entries["defaultInterfaceLanguage"]);
    m_strDefaultOutputLanguage    = Utf8ToWide(entries["defaultOutputLanguage"]);

    if (entries.find("showDataViewer") != entries.end())
        m_menuVisibility.m_bShowDataViewer = (entries["showDataViewer"] == "true") ? TRUE : FALSE;
    if (entries.find("showTransform") != entries.end())
        m_menuVisibility.m_bShowTransform  = (entries["showTransform"] == "true") ? TRUE : FALSE;
    if (entries.find("showExport") != entries.end())
        m_menuVisibility.m_bShowExport     = (entries["showExport"] == "true") ? TRUE : FALSE;
    if (entries.find("showHistory") != entries.end())
        m_menuVisibility.m_bShowHistory    = (entries["showHistory"] == "true") ? TRUE : FALSE;
    if (entries.find("showWorkflow") != entries.end())
        m_menuVisibility.m_bShowWorkflow   = (entries["showWorkflow"] == "true") ? TRUE : FALSE;
    if (entries.find("showWebextract") != entries.end())
        m_menuVisibility.m_bShowWebextract = (entries["showWebextract"] == "true") ? TRUE : FALSE;
    if (entries.find("showSettings") != entries.end())
        m_menuVisibility.m_bShowSettings   = (entries["showSettings"] == "true") ? TRUE : FALSE;

    m_arrPlugins.clear();
    for (std::map<std::string, std::string>::const_iterator it = entries.begin();
         it != entries.end(); ++it)
    {
        const std::string& strKey = it->first;
        if (strKey.substr(0, 7) == "plugin_")
        {
            PluginConfig cfg;
            cfg.m_strPluginId = Utf8ToWide(strKey.substr(7));
            cfg.m_bEnabled    = (it->second == "true") ? TRUE : FALSE;
            m_arrPlugins.push_back(cfg);
        }
    }

    return TRUE;
}

void SolutionProfile::ParseProfileJson(
    const std::string& strJson,
    std::map<std::string, std::string>& outEntries) const
{
    std::istringstream ss(strJson);
    std::string strLine;

    while (std::getline(ss, strLine))
    {
        size_t nColon = strLine.find(':');
        if (nColon == std::string::npos)
            continue;

        std::string strKey = strLine.substr(0, nColon);
        std::string strVal = strLine.substr(nColon + 1);

        std::string strCleanKey;
        for (char c : strKey)
        {
            if (c != ' ' && c != '"' && c != '{' && c != '\t' && c != '\r')
                strCleanKey += c;
        }

        std::string strCleanVal;
        for (char c : strVal)
        {
            if (c != ' ' && c != '"' && c != '}' && c != ',' && c != '\t' && c != '\r')
                strCleanVal += c;
        }

        if (!strCleanKey.empty() && !strCleanVal.empty())
            outEntries[strCleanKey] = strCleanVal;
    }
}

const CString& SolutionProfile::GetProfileId() const
{
    return m_strProfileId;
}

const CString& SolutionProfile::GetProfileName() const
{
    return m_strProfileName;
}

const CString& SolutionProfile::GetDefaultInterfaceLanguage() const
{
    return m_strDefaultInterfaceLanguage;
}

const CString& SolutionProfile::GetDefaultOutputLanguage() const
{
    return m_strDefaultOutputLanguage;
}

const MenuVisibility& SolutionProfile::GetMenuVisibility() const
{
    return m_menuVisibility;
}

void SolutionProfile::SetPluginEnabled(const CString& strPluginId, BOOL bEnabled)
{
    for (int i = 0; i < static_cast<int>(m_arrPlugins.size()); ++i)
    {
        if (m_arrPlugins[i].m_strPluginId == strPluginId)
        {
            m_arrPlugins[i].m_bEnabled = bEnabled;
            return;
        }
    }

    PluginConfig cfg;
    cfg.m_strPluginId = strPluginId;
    cfg.m_bEnabled    = bEnabled;
    m_arrPlugins.push_back(cfg);
}

BOOL SolutionProfile::IsPluginEnabled(const CString& strPluginId) const
{
    for (int i = 0; i < static_cast<int>(m_arrPlugins.size()); ++i)
    {
        if (m_arrPlugins[i].m_strPluginId == strPluginId)
            return m_arrPlugins[i].m_bEnabled;
    }
    return TRUE;
}
