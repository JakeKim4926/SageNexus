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
    const CString& GetRunningJobId() const;
    const CString& GetCurrentStepName() const;

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
    volatile BOOL             m_bQueueRunning;
    volatile BOOL             m_bCancelRequested;
    CString                   m_strRunningJobId;
    WorkflowService           m_workflowService;
};
