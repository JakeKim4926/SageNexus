#pragma once
#include "../pch.h"

// 활성화된 기능 모듈 목록
struct PluginConfig
{
    CString m_strPluginId;
    BOOL    m_bEnabled;
};

// 숨길 메뉴/페이지 설정
struct MenuVisibility
{
    BOOL m_bShowDataViewer;
    BOOL m_bShowTransform;
    BOOL m_bShowExport;
    BOOL m_bShowHistory;
    BOOL m_bShowSettings;

    MenuVisibility()
        : m_bShowDataViewer(TRUE)
        , m_bShowTransform(TRUE)
        , m_bShowExport(TRUE)
        , m_bShowHistory(TRUE)
        , m_bShowSettings(TRUE)
    {}
};

// 회사별 기능 조합을 정의하는 프로필
// 코어는 프로필에 따라 활성 플러그인과 UI 구성을 결정한다
class SolutionProfile
{
public:
    SolutionProfile();

    BOOL LoadFromFile(const CString& strFilePath, CString& strError);

    const CString&         GetProfileId() const;
    const CString&         GetProfileName() const;
    const CString&         GetDefaultInterfaceLanguage() const;
    const CString&         GetDefaultOutputLanguage() const;
    const MenuVisibility&  GetMenuVisibility() const;

    BOOL IsPluginEnabled(const CString& strPluginId) const;

    // Phase 1: 기본 프로필 (모든 메뉴 활성)
    void SetDefault();

private:
    CString                  m_strProfileId;
    CString                  m_strProfileName;
    CString                  m_strDefaultInterfaceLanguage;
    CString                  m_strDefaultOutputLanguage;
    MenuVisibility           m_menuVisibility;
    std::vector<PluginConfig> m_arrPlugins;
};
