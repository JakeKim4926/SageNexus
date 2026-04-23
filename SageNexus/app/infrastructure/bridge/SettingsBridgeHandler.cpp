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

    dispatcher.RegisterHandler(L"settings.language", L"getInterfaceLanguage",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleGetInterfaceLanguage(msg);
        });

    dispatcher.RegisterHandler(L"settings.language", L"setInterfaceLanguage",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleSetInterfaceLanguage(msg);
        });

    dispatcher.RegisterHandler(L"settings.security", L"changePassword",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleChangePassword(msg);
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
        L"\"showWorkflow\":%s,"
        L"\"showWebextract\":%s,"
        L"\"showSettings\":%s"
        L"}}",
        (LPCWSTR)profile.GetProfileId(),
        (LPCWSTR)profile.GetProfileName(),
        (LPCWSTR)profile.GetDefaultInterfaceLanguage(),
        (LPCWSTR)profile.GetDefaultOutputLanguage(),
        vis.m_bShowDataViewer  ? L"true" : L"false",
        vis.m_bShowTransform   ? L"true" : L"false",
        vis.m_bShowExport      ? L"true" : L"false",
        vis.m_bShowHistory     ? L"true" : L"false",
        vis.m_bShowWorkflow    ? L"true" : L"false",
        vis.m_bShowWebextract  ? L"true" : L"false",
        vis.m_bShowSettings    ? L"true" : L"false"
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
    CString strPluginId = JsonExtractString(msg.m_strPayloadJson, L"pluginId");
    BOOL bEnabled       = JsonExtractBool(msg.m_strPayloadJson, L"enabled");

    if (strPluginId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"pluginId is required\"}}";
    }

    sageMgr.GetProfile().SetPluginEnabled(strPluginId, bEnabled);

    CString strSaveError;
    if (!sageMgr.SaveProfile(strSaveError))
    {
        CString strReloadError;
        sageMgr.ReloadProfile(strReloadError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"PROFILE_SAVE_FAILED\",\"message\":\"" +
               JsonEscapeString(strSaveError) + L"\"}}";
    }

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
    CString strLang = JsonExtractString(msg.m_strPayloadJson, L"outputLanguage");
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

CString SettingsBridgeHandler::HandleGetInterfaceLanguage(const BridgeMessage& msg)
{
    CString strLang = sageMgr.GetConfigStore().GetString(L"interfaceLanguage", L"ko");
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"interfaceLanguage\":\"" + strLang + L"\"}}";
}

CString SettingsBridgeHandler::HandleSetInterfaceLanguage(const BridgeMessage& msg)
{
    CString strLang = JsonExtractString(msg.m_strPayloadJson, L"interfaceLanguage");
    if (strLang != L"ko" && strLang != L"en")
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\","
               L"\"message\":\"interfaceLanguage must be 'ko' or 'en'\"}}";
    }

    sageMgr.GetConfigStore().SetString(L"interfaceLanguage", strLang);
    sageMgr.GetConfigStore().Save();

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"interfaceLanguage\":\"" + strLang + L"\"}}";
}

CString SettingsBridgeHandler::HandleChangePassword(const BridgeMessage& msg)
{
    CString strOldPassword = JsonExtractString(msg.m_strPayloadJson, L"oldPassword");
    CString strNewPassword = JsonExtractString(msg.m_strPayloadJson, L"newPassword");

    if (strNewPassword.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\","
               L"\"message\":\"newPassword is required\"}}";
    }

    ProfileSecurity& security = sageMgr.GetSecurity();
    CString strError;
    if (!security.ChangePassword(strOldPassword, strNewPassword, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"CHANGE_PASSWORD_FAILED\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{}}";
}
