#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/domain/model/DataTable.h"

class ImportBridgeHandler
{
public:
    ImportBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, HWND hParentWnd, DataTable* pSharedTable);

private:
    CString HandleOpenFileDialog(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleLoadFile(const BridgeMessage& msg);

    CString SerializeTableToJson(const DataTable& table, const CString& strTableId) const;

    DataTable* m_pSharedTable;
    int        m_nTableIdCounter;
};
