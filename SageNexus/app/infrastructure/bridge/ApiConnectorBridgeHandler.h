#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/ApiConnectorService.h"

class ApiConnectorBridgeHandler
{
public:
    ApiConnectorBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher);

private:
    CString HandleGetConnectors(const BridgeMessage& msg);
    CString HandleAddConnector(const BridgeMessage& msg);
    CString HandleRemoveConnector(const BridgeMessage& msg);
    CString HandleUpdateConnector(const BridgeMessage& msg);
    CString HandleTestConnector(const BridgeMessage& msg);

    CString SerializeConnector(const ApiConnector& conn) const;
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString EscapeJson(const CString& str) const;

    ApiConnectorService m_service;
};
