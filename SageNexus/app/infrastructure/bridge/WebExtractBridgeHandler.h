#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/WebExtractService.h"
#include "app/domain/model/DataTable.h"

class WebExtractBridgeHandler
{
public:
    WebExtractBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, DataTable* pCurrentTable);

private:
    CString HandleFetchAndExtract(const BridgeMessage& msg);
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString EscapeJson(const CString& str) const;

    WebExtractService  m_service;
    DataTable*         m_pCurrentTable;
};
