#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/ApiCallService.h"

class ApiCallBridgeHandler
{
public:
    void RegisterHandlers(BridgeDispatcher& dispatcher);

private:
    CString HandleCallApi(const BridgeMessage& msg);
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString EscapeJson(const CString& str) const;

    ApiCallService m_service;
};
