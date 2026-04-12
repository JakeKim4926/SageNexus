#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/domain/model/DataTable.h"

class ExportBridgeHandler
{
public:
    ExportBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, HWND hParentWnd, DataTable* pSharedTable);

private:
    CString HandleExportCsv(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleExportXlsx(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleExportHtml(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleExportWord(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleExportPdf(const BridgeMessage& msg, HWND hParentWnd);
    CString HandleGetArtifacts(const BridgeMessage& msg);

    HWND       m_hParentWnd;
    DataTable* m_pSharedTable;
};
