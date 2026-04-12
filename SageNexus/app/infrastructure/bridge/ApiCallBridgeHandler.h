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

    ApiCallService m_service;
};
