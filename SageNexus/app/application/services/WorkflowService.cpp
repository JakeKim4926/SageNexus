#include "pch.h"
#include "app/application/services/WorkflowService.h"
#include "app/application/SageApp.h"
#include "Define.h"

WorkflowService::WorkflowService()
    : m_bRunning(FALSE)
    , m_bCancelRequested(FALSE)
{
}

BOOL WorkflowService::GetWorkflows(std::vector<WorkflowDefinition>& arrWorkflows, CString& strError)
{
    return m_store.LoadWorkflows(arrWorkflows, strError);
}

BOOL WorkflowService::GetWorkflow(const CString& strId, WorkflowDefinition& outWorkflow, CString& strError)
{
    std::vector<WorkflowDefinition> arrWorkflows;
    if (!m_store.LoadWorkflows(arrWorkflows, strError))
        return FALSE;

    for (const WorkflowDefinition& wf : arrWorkflows)
    {
        if (wf.m_strId == strId)
        {
            outWorkflow = wf;
            return TRUE;
        }
    }

    strError = L"Workflow를 찾을 수 없습니다: " + strId;
    return FALSE;
}

BOOL WorkflowService::SaveWorkflow(WorkflowDefinition& workflow, CString& strError)
{
    return m_store.SaveWorkflow(workflow, strError);
}

BOOL WorkflowService::DeleteWorkflow(const CString& strId, CString& strError)
{
    return m_store.DeleteWorkflow(strId, strError);
}

BOOL WorkflowService::RunWorkflow(const CString& strId, HWND hNotifyWnd, CString& strError)
{
    if (m_bRunning)
    {
        strError = L"이미 실행 중인 Workflow가 있습니다.";
        return FALSE;
    }

    WorkflowDefinition workflow;
    if (!GetWorkflow(strId, workflow, strError))
        return FALSE;

    if (workflow.m_arrSteps.empty())
    {
        strError = L"실행할 Step이 없습니다.";
        return FALSE;
    }

    m_bCancelRequested = FALSE;
    m_strLastError     = L"";

    RunContext* pCtx  = new RunContext();
    pCtx->pService    = this;
    pCtx->workflow    = workflow;
    pCtx->hNotifyWnd  = hNotifyWnd;

    HANDLE hThread = CreateThread(nullptr, 0, RunThread, pCtx, 0, nullptr);
    if (!hThread)
    {
        delete pCtx;
        strError = L"워커 스레드 생성 실패";
        return FALSE;
    }

    CloseHandle(hThread);
    return TRUE;
}

void WorkflowService::CancelWorkflow()
{
    m_bCancelRequested = TRUE;
}

BOOL WorkflowService::IsRunning() const
{
    return m_bRunning;
}

const CString& WorkflowService::GetLastError() const
{
    return m_strLastError;
}

DWORD WINAPI WorkflowService::RunThread(LPVOID pParam)
{
    RunContext* pCtx = reinterpret_cast<RunContext*>(pParam);
    pCtx->pService->ExecuteSteps(pCtx->workflow, pCtx->hNotifyWnd);
    delete pCtx;
    return 0;
}

void WorkflowService::ExecuteSteps(const WorkflowDefinition& workflow, HWND hNotifyWnd)
{
    m_bRunning = TRUE;

    int nTotal  = (int)workflow.m_arrSteps.size();
    BOOL bSuccess = TRUE;

    for (int i = 0; i < nTotal; ++i)
    {
        if (m_bCancelRequested)
        {
            m_strLastError = L"사용자에 의해 취소되었습니다.";
            bSuccess = FALSE;
            break;
        }

        PostMessage(hNotifyWnd, WM_WORKFLOW_PROGRESS, (WPARAM)(i + 1), (LPARAM)nTotal);

        sageMgr.GetLogger().Log(
            LogLevel::Info,
            L"[Workflow] " + workflow.m_strName +
            L" — step " + workflow.m_arrSteps[i].m_strStepType +
            L" (" + workflow.m_arrSteps[i].m_strName + L")");
    }

    ExecutionRecord record;
    record.m_strOperationType = L"workflow";
    record.m_strSourceName    = workflow.m_strName;
    record.m_bSuccess         = bSuccess;
    record.m_strErrorMessage  = m_strLastError;

    CString strSaveError;
    m_historyStore.SaveRecord(record, strSaveError);

    m_bRunning = FALSE;

    PostMessage(hNotifyWnd, WM_WORKFLOW_COMPLETE, (WPARAM)bSuccess, 0);
}
