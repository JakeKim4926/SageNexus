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
    BOOL m_bShowWorkflow;
    BOOL m_bShowWebextract;
    BOOL m_bShowSettings;

    MenuVisibility()
        : m_bShowDataViewer(TRUE)
        , m_bShowTransform(TRUE)
        , m_bShowExport(TRUE)
        , m_bShowHistory(TRUE)
        , m_bShowWorkflow(TRUE)
        , m_bShowWebextract(TRUE)
        , m_bShowSettings(TRUE)
    {}
};

class SolutionProfile
{
public:
    SolutionProfile();

    BOOL LoadFromFile(const CString& strPath, CString& strError);
    BOOL LoadFromResource(int nResourceId, CString& strError);
    BOOL SaveToFile(const CString& strPath, CString& strError) const;

    const CString&         GetProfileId() const;
    const CString&         GetProfileName() const;
    const CString&         GetDefaultInterfaceLanguage() const;
    const CString&         GetDefaultOutputLanguage() const;
    const MenuVisibility&  GetMenuVisibility() const;

    BOOL IsPluginEnabled(const CString& strPluginId) const;
    void SetPluginEnabled(const CString& strPluginId, BOOL bEnabled);

    void SetDefault();

private:
    std::string BuildJsonString() const;
    BOOL LoadFromJsonString(const std::string& strJson, CString& strError);
    void ParseProfileJson(
        const std::string& strJson,
        std::map<std::string, std::string>& outEntries) const;
    void ParseMenuArray(const std::string& strJson);
    void ParsePluginArray(const std::string& strJson);

    CString                   m_strProfileId;
    CString                   m_strProfileName;
    CString                   m_strDefaultInterfaceLanguage;
    CString                   m_strDefaultOutputLanguage;
    MenuVisibility            m_menuVisibility;
    std::vector<PluginConfig> m_arrPlugins;
};
