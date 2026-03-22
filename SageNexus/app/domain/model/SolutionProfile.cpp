#include "pch.h"
#include "SolutionProfile.h"

SolutionProfile::SolutionProfile()
{
    SetDefault();
}

void SolutionProfile::SetDefault()
{
    m_strProfileId                   = L"default";
    m_strProfileName                 = L"Default Profile";
    m_strDefaultInterfaceLanguage    = L"ko";
    m_strDefaultOutputLanguage       = L"ko";
    // MenuVisibility 기본 생성자에서 모두 TRUE로 설정됨
}

BOOL SolutionProfile::LoadFromFile(const CString& strFilePath, CString& strError)
{
    // Phase 3에서 플러그인 런타임과 함께 구현
    strError = L"Profile file loading is not yet implemented.";
    return FALSE;
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

BOOL SolutionProfile::IsPluginEnabled(const CString& strPluginId) const
{
    for (int i = 0; i < static_cast<int>(m_arrPlugins.size()); ++i)
    {
        if (m_arrPlugins[i].m_strPluginId == strPluginId)
            return m_arrPlugins[i].m_bEnabled;
    }
    return FALSE;
}
