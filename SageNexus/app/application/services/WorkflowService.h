#pragma once
#include "pch.h"
#include "app/domain/model/WorkflowDefinition.h"
#include "app/infrastructure/workflow/WorkflowStore.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include <vector>

class WorkflowService
{
public:
    WorkflowService();

    BOOL GetWorkflows(std::vector<WorkflowDefinition>& arrWorkflows, CString& strError);
    BOOL GetWorkflow(const CString& strId, WorkflowDefinition& outWorkflow, CString& strError);
    BOOL SaveWorkflow(WorkflowDefinition& workflow, CString& strError);
    BOOL DeleteWorkflow(const CString& strId, CString& strError);

    BOOL RunWorkflow(const CString& strId, HWND hNotifyWnd, CString& strError);
    void CancelWorkflow();

    BOOL           IsRunning() const;
    const CString& GetLastError() const;

private:
    struct RunContext
    {
        WorkflowService*   pService;
        WorkflowDefinition workflow;
        HWND               hNotifyWnd;
    };

    static DWORD WINAPI RunThread(LPVOID pParam);
    void ExecuteSteps(const WorkflowDefinition& workflow, HWND hNotifyWnd);

    WorkflowStore          m_store;
    ExecutionHistoryStore  m_historyStore;
    BOOL                   m_bRunning;
    volatile BOOL          m_bCancelRequested;
    CString                m_strLastError;
};
