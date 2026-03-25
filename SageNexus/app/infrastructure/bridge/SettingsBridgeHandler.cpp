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
