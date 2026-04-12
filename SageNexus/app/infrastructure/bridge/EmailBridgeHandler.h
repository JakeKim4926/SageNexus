#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/EmailService.h"

class EmailBridgeHandler
{
public:
    void RegisterHandlers(BridgeDispatcher& dispatcher);

private:
    CString HandleSendEmail(const BridgeMessage& msg);

    EmailService m_service;
};
