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
};
