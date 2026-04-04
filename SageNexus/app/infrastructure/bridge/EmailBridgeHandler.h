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
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString EscapeJson(const CString& str) const;

    EmailService m_service;
};
