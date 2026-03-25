#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"

class SettingsBridgeHandler
{
public:
    SettingsBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher);

private:
    CString HandleGetProfile(const BridgeMessage& msg);
    CString HandleGetPlugins(const BridgeMessage& msg);
    CString HandleTogglePlugin(const BridgeMessage& msg);

    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    BOOL    ExtractPayloadBool(const CString& strJson, const CString& strKey) const;
};
