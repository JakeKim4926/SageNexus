#pragma once
#include "pch.h"
#include "app/domain/model/ExecutionJob.h"
#include "app/application/services/WorkflowService.h"
#include <vector>

class JobQueueService
{
public:
    JobQueueService();
    ~JobQueueService();

    BOOL EnqueueJob(const CString& strWorkflowId, const CString& strWorkflowName, HWND hNotifyWnd, CString& strError);
    void GetQueue(std::vector<ExecutionJob>& arrJobs) const;
    void CancelJob(const CString& strJobId);
    void CancelRunningJob();
    BOOL RetryJob(const CString& strJobId, HWND hNotifyWnd, CString& strError);
    CString        GetRunningJobId() const;  // 락으로 보호하므로 값 반환
    const CString& GetCurrentStepName() const;
    const DataTable& GetLastOutputTable() const;

private:
    struct QueueContext
    {
        JobQueueService* pService;
        HWND             hNotifyWnd;
    };

    static DWORD WINAPI QueueThread(LPVOID pParam);
    void ProcessJobs(HWND hNotifyWnd);
    CString GenerateJobId() const;

    std::vector<ExecutionJob> m_arrJobs;
    CRITICAL_SECTION          m_cs;
    // m_bQueueRunning / m_bCancelRequested: LONG + Interlocked 으로 원자 접근
    volatile LONG             m_bQueueRunning;
    volatile LONG             m_bCancelRequested;
    // m_strRunningJobId: m_cs 안에서만 접근
    CString                   m_strRunningJobId;
    WorkflowService           m_workflowService;
};
