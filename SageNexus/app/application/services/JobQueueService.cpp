#include "pch.h"
#include "app/application/services/JobQueueService.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <vector>

JobQueueService::JobQueueService()
    : m_bQueueRunning(FALSE)
    , m_bCancelRequested(FALSE)
{
    InitializeCriticalSection(&m_cs);
}

JobQueueService::~JobQueueService()
{
    DeleteCriticalSection(&m_cs);
}

BOOL JobQueueService::EnqueueJob(const CString& strWorkflowId, const CString& strWorkflowName, HWND hNotifyWnd, CString& strError)
{
    if (strWorkflowId.IsEmpty())
    {
        strError = L"workflowId가 비어 있습니다.";
        return FALSE;
    }

    ExecutionJob job;
    job.m_strJobId        = GenerateJobId();
    job.m_strWorkflowId   = strWorkflowId;
    job.m_strWorkflowName = strWorkflowName;
    job.m_eStatus         = JobStatus::Pending;

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    job.m_strCreatedAt.Format(L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    EnterCriticalSection(&m_cs);
    m_arrJobs.push_back(job);
    LeaveCriticalSection(&m_cs);

    PostMessage(hNotifyWnd, WM_JOB_QUEUE_CHANGED, 0, 0);

    // m_bQueueRunning은 Interlocked 읽기로 확인한다.
    if (InterlockedCompareExchange(&m_bQueueRunning, 0, 0) == 0)
    {
        InterlockedExchange(&m_bCancelRequested, FALSE);

        QueueContext* pCtx = new QueueContext();
        pCtx->pService     = this;
        pCtx->hNotifyWnd   = hNotifyWnd;

        HANDLE hThread = CreateThread(nullptr, 0, QueueThread, pCtx, 0, nullptr);
        if (!hThread)
        {
            delete pCtx;
            strError = L"큐 워커 스레드 생성 실패";
            return FALSE;
        }
        CloseHandle(hThread);
    }

    return TRUE;
}

void JobQueueService::GetQueue(std::vector<ExecutionJob>& arrJobs) const
{
    EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
    arrJobs = m_arrJobs;
    LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
}

void JobQueueService::CancelJob(const CString& strJobId)
{
    EnterCriticalSection(&m_cs);
    for (ExecutionJob& job : m_arrJobs)
    {
        if (job.m_strJobId == strJobId)
        {
            if (job.m_eStatus == JobStatus::Pending)
                job.m_eStatus = JobStatus::Cancelled;
            else if (job.m_eStatus == JobStatus::Running)
                InterlockedExchange(&m_bCancelRequested, TRUE);
            break;
        }
    }
    LeaveCriticalSection(&m_cs);
}

void JobQueueService::CancelRunningJob()
{
    EnterCriticalSection(&m_cs);
    CString strRunningId = m_strRunningJobId;
    LeaveCriticalSection(&m_cs);

    if (!strRunningId.IsEmpty())
        CancelJob(strRunningId);
}

BOOL JobQueueService::RetryJob(const CString& strJobId, HWND hNotifyWnd, CString& strError)
{
    if (strJobId.IsEmpty())
    {
        strError = L"jobId가 비어 있습니다.";
        return FALSE;
    }

    CString strWorkflowId;
    CString strWorkflowName;

    EnterCriticalSection(&m_cs);
    for (ExecutionJob& job : m_arrJobs)
    {
        if (job.m_strJobId == strJobId)
        {
            if (job.m_eStatus != JobStatus::Failed && job.m_eStatus != JobStatus::Cancelled)
            {
                LeaveCriticalSection(&m_cs);
                strError = L"실패 또는 취소된 Job만 재실행할 수 있습니다.";
                return FALSE;
            }
            strWorkflowId   = job.m_strWorkflowId;
            strWorkflowName = job.m_strWorkflowName;
            break;
        }
    }
    LeaveCriticalSection(&m_cs);

    if (strWorkflowId.IsEmpty())
    {
        strError = L"해당 jobId를 찾을 수 없습니다.";
        return FALSE;
    }

    return EnqueueJob(strWorkflowId, strWorkflowName, hNotifyWnd, strError);
}

CString JobQueueService::GetRunningJobId() const
{
    EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
    CString strId = m_strRunningJobId;
    LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_cs));
    return strId;
}

const CString& JobQueueService::GetCurrentStepName() const
{
    return m_workflowService.GetCurrentStepName();
}

const DataTable& JobQueueService::GetLastOutputTable() const
{
    return m_workflowService.GetLastOutputTable();
}

DWORD WINAPI JobQueueService::QueueThread(LPVOID pParam)
{
    QueueContext* pCtx = reinterpret_cast<QueueContext*>(pParam);
    pCtx->pService->ProcessJobs(pCtx->hNotifyWnd);
    delete pCtx;
    return 0;
}

void JobQueueService::ProcessJobs(HWND hNotifyWnd)
{
    InterlockedExchange(&m_bQueueRunning, TRUE);

    while (TRUE)
    {
        CString strNextJobId;
        CString strWorkflowId;

        EnterCriticalSection(&m_cs);
        for (ExecutionJob& job : m_arrJobs)
        {
            if (job.m_eStatus == JobStatus::Pending)
            {
                job.m_eStatus = JobStatus::Running;
                strNextJobId  = job.m_strJobId;
                strWorkflowId = job.m_strWorkflowId;
                break;
            }
        }
        LeaveCriticalSection(&m_cs);

        if (strNextJobId.IsEmpty())
            break;

        EnterCriticalSection(&m_cs);
        m_strRunningJobId = strNextJobId;
        LeaveCriticalSection(&m_cs);

        InterlockedExchange(&m_bCancelRequested, FALSE);

        PostMessage(hNotifyWnd, WM_JOB_QUEUE_CHANGED, 0, 0);

        sageMgr.GetLogger().Log(LogLevel::Info,
            L"[JobQueue] Job 시작: " + strNextJobId + L" / workflow: " + strWorkflowId);

        CString strError;
        BOOL bSuccess = m_workflowService.RunSync(
            strWorkflowId,
            reinterpret_cast<volatile BOOL&>(m_bCancelRequested),
            hNotifyWnd,
            strError);

        JobStatus eResult = bSuccess ? JobStatus::Completed : JobStatus::Failed;

        EnterCriticalSection(&m_cs);
        for (ExecutionJob& job : m_arrJobs)
        {
            if (job.m_strJobId == strNextJobId)
            {
                job.m_eStatus        = eResult;
                job.m_strErrorMessage = strError;
                break;
            }
        }
        m_strRunningJobId = L"";
        LeaveCriticalSection(&m_cs);

        sageMgr.GetLogger().Log(LogLevel::Info,
            L"[JobQueue] Job 완료: " + strNextJobId +
            L" / " + (bSuccess ? CString(L"성공") : CString(L"실패: ") + strError));

        PostMessage(hNotifyWnd, WM_JOB_QUEUE_CHANGED, 0, 0);
    }

    InterlockedExchange(&m_bQueueRunning, FALSE);
}

CString JobQueueService::GenerateJobId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"JOB-%04d%02d%02d-%02d%02d%02d-%04d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond,
        st.wMilliseconds);
    return strId;
}
