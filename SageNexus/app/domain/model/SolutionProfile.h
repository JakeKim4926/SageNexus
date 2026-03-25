#pragma once
#include "pch.h"

struct PluginConfig
{
    CString m_strPluginId;
    BOOL    m_bEnabled;
};

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
    void SetPluginEnabled(const CString& strPluginId, BOOL bEnabled);

    void SetDefault();

private:
    void ParseProfileJson(
        const std::string& strJson,
        std::map<std::string, std::string>& outEntries) const;

    CString                   m_strProfileId;
    CString                   m_strProfileName;
    CString                   m_strDefaultInterfaceLanguage;
    CString                   m_strDefaultOutputLanguage;
    MenuVisibility            m_menuVisibility;
    std::vector<PluginConfig> m_arrPlugins;
};
