#pragma once
#include "pch.h"
#include "WebViewHost.h"
#include "app/domain/model/DataTable.h"
#include "app/infrastructure/bridge/ImportBridgeHandler.h"
#include "app/infrastructure/bridge/TransformBridgeHandler.h"
#include "app/infrastructure/bridge/ExportBridgeHandler.h"
#include "app/infrastructure/bridge/HistoryBridgeHandler.h"
#include "app/infrastructure/bridge/SettingsBridgeHandler.h"
#include "app/infrastructure/bridge/WorkflowBridgeHandler.h"
#include "app/infrastructure/bridge/WebExtractBridgeHandler.h"
#include "app/infrastructure/bridge/EmailBridgeHandler.h"
#include "app/infrastructure/bridge/ApiCallBridgeHandler.h"
#include "app/infrastructure/bridge/JobQueueBridgeHandler.h"
#include "app/infrastructure/bridge/SchedulerBridgeHandler.h"
#include "app/infrastructure/bridge/ApiConnectorBridgeHandler.h"

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
    void OnWorkflowProgress(int nStep, int nTotal);
    void OnWorkflowComplete(BOOL bSuccess);
    void OnJobQueueChanged();
    void OnSchedulerTick();
    void OnDestroy();

    void NavigateToShell();
    void RegisterBridgeHandlers();
    void UpdateCurrentTableFromWorkflow();

    HWND                    m_hWnd;
    WebViewHost*            m_pWebViewHost;
    DataTable               m_currentTable;
    ImportBridgeHandler     m_importBridgeHandler;
    TransformBridgeHandler  m_transformBridgeHandler;
    ExportBridgeHandler     m_exportBridgeHandler;
    HistoryBridgeHandler    m_historyBridgeHandler;
    SettingsBridgeHandler   m_settingsBridgeHandler;
    WorkflowBridgeHandler     m_workflowBridgeHandler;
    WebExtractBridgeHandler   m_webExtractBridgeHandler;
    EmailBridgeHandler        m_emailBridgeHandler;
    ApiCallBridgeHandler      m_apiCallBridgeHandler;
    JobQueueBridgeHandler     m_jobQueueBridgeHandler;
    SchedulerBridgeHandler        m_schedulerBridgeHandler;
    ApiConnectorBridgeHandler     m_apiConnectorBridgeHandler;
};
