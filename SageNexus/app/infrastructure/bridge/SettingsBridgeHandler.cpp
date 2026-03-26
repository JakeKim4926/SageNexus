#include "pch.h"
#include "app/infrastructure/bridge/SettingsBridgeHandler.h"
#include "app/application/SageApp.h"

SettingsBridgeHandler::SettingsBridgeHandler()
{
}

void SettingsBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"settings.profile", L"getProfile",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetProfile(msg);
        });

    dispatcher.RegisterHandler(L"settings.plugins", L"getPlugins",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetPlugins(msg);
        });

    dispatcher.RegisterHandler(L"settings.plugins", L"togglePlugin",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleTogglePlugin(msg);
        });

    dispatcher.RegisterHandler(L"settings.language", L"getOutputLanguage",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetOutputLanguage(msg);
        });

    dispatcher.RegisterHandler(L"settings.language", L"setOutputLanguage",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleSetOutputLanguage(msg);
        });
}

CString SettingsBridgeHandler::HandleGetProfile(const BridgeMessage& msg)
{
    const SolutionProfile& profile = sageMgr.GetProfile();
    const MenuVisibility&  vis     = profile.GetMenuVisibility();

    CString strPayload;
    strPayload.Format(
        L"{"
        L"\"profileId\":\"%s\","
        L"\"profileName\":\"%s\","
        L"\"defaultInterfaceLanguage\":\"%s\","
        L"\"defaultOutputLanguage\":\"%s\","
        L"\"menuVisibility\":{"
        L"\"showDataViewer\":%s,"
        L"\"showTransform\":%s,"
        L"\"showExport\":%s,"
        L"\"showHistory\":%s,"
        L"\"showSettings\":%s"
        L"}}",
        (LPCWSTR)profile.GetProfileId(),
        (LPCWSTR)profile.GetProfileName(),
        (LPCWSTR)profile.GetDefaultInterfaceLanguage(),
        (LPCWSTR)profile.GetDefaultOutputLanguage(),
        vis.m_bShowDataViewer ? L"true" : L"false",
        vis.m_bShowTransform  ? L"true" : L"false",
        vis.m_bShowExport     ? L"true" : L"false",
        vis.m_bShowHistory    ? L"true" : L"false",
        vis.m_bShowSettings   ? L"true" : L"false"
    );

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strPayload + L"}";
}

CString SettingsBridgeHandler::HandleGetPlugins(const BridgeMessage& msg)
{
    const PluginManager& pm = sageMgr.GetPluginManager();
    const std::vector<PluginEntry>& arrPlugins = pm.GetAllPlugins();
    const SolutionProfile& profile = sageMgr.GetProfile();

    CString strArray = L"[";
    for (int i = 0; i < static_cast<int>(arrPlugins.size()); ++i)
    {
        if (i > 0)
            strArray += L",";

        BOOL bEnabled = profile.IsPluginEnabled(arrPlugins[i].m_strPluginId);
        CString strEntry;
        strEntry.Format(L"{\"pluginId\":\"%s\",\"pluginName\":\"%s\",\"enabled\":%s}",
            (LPCWSTR)arrPlugins[i].m_strPluginId,
            (LPCWSTR)arrPlugins[i].m_strPluginName,
            bEnabled ? L"true" : L"false");
        strArray += strEntry;
    }
    strArray += L"]";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strArray + L"}";
}

CString SettingsBridgeHandler::HandleTogglePlugin(const BridgeMessage& msg)
{
    CString strPluginId = ExtractPayloadString(msg.m_strPayloadJson, L"pluginId");
    BOOL bEnabled       = ExtractPayloadBool(msg.m_strPayloadJson, L"enabled");

    if (strPluginId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"pluginId is required\"}}";
    }

    sageMgr.GetProfile().SetPluginEnabled(strPluginId, bEnabled);
    sageMgr.SaveProfileFile();

    CString strResult;
    strResult.Format(L"{\"pluginId\":\"%s\",\"enabled\":%s}",
        (LPCWSTR)strPluginId,
        bEnabled ? L"true" : L"false");

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strResult + L"}";
}

CString SettingsBridgeHandler::HandleGetOutputLanguage(const BridgeMessage& msg)
{
    CString strLang = sageMgr.GetConfigStore().GetString(L"outputLanguage", L"ko");
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"outputLanguage\":\"" + strLang + L"\"}}";
}

CString SettingsBridgeHandler::HandleSetOutputLanguage(const BridgeMessage& msg)
{
    CString strLang = ExtractPayloadString(msg.m_strPayloadJson, L"outputLanguage");
    if (strLang != L"ko" && strLang != L"en")
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\","
               L"\"message\":\"outputLanguage must be 'ko' or 'en'\"}}";
    }

    sageMgr.GetConfigStore().SetString(L"outputLanguage", strLang);
    sageMgr.GetConfigStore().Save();

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"outputLanguage\":\"" + strLang + L"\"}}";
}

CString SettingsBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos)
        return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return L"";

    size_t nQuoteOpen = json.find('"', nColon + 1);
    if (nQuoteOpen == std::string::npos)
        return L"";

    size_t nQuoteClose = json.find('"', nQuoteOpen + 1);
    if (nQuoteClose == std::string::npos)
        return L"";

    std::string strVal = json.substr(nQuoteOpen + 1, nQuoteClose - nQuoteOpen - 1);
    return Utf8ToWide(strVal);
}

BOOL SettingsBridgeHandler::ExtractPayloadBool(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
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

    if (json.substr(nStart, 4) == "true")
        return TRUE;

    return FALSE;
}
