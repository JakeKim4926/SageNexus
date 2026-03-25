#pragma once
#include "pch.h"
#include "WebViewHost.h"
#include "app/domain/model/DataTable.h"
#include "app/infrastructure/bridge/ImportBridgeHandler.h"
#include "app/infrastructure/bridge/TransformBridgeHandler.h"
#include "app/infrastructure/bridge/ExportBridgeHandler.h"
#include "app/infrastructure/bridge/HistoryBridgeHandler.h"
#include "app/infrastructure/bridge/SettingsBridgeHandler.h"

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    BOOL Create(int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnCreate();
    void OnSize(int nWidth, int nHeight);
    void OnGetMinMaxInfo(MINMAXINFO* pInfo) const;
    void OnWebViewReady(BOOL bSuccess);
    void OnDestroy();

    void NavigateToShell();
    void RegisterBridgeHandlers();

    HWND                    m_hWnd;
    WebViewHost*            m_pWebViewHost;
    DataTable               m_currentTable;
    ImportBridgeHandler     m_importBridgeHandler;
    TransformBridgeHandler  m_transformBridgeHandler;
    ExportBridgeHandler     m_exportBridgeHandler;
    HistoryBridgeHandler    m_historyBridgeHandler;
    SettingsBridgeHandler   m_settingsBridgeHandler;
};
