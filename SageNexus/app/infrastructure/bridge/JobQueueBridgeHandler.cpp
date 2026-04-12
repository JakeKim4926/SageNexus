#include "pch.h"
#include "app/infrastructure/bridge/JobQueueBridgeHandler.h"
#include "app/application/SageApp.h"
#include <string>
#include <vector>

JobQueueBridgeHandler::JobQueueBridgeHandler()
    : m_hMainWnd(nullptr)
{
}

void JobQueueBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, HWND hMainWnd)
{
    m_hMainWnd = hMainWnd;

    dispatcher.RegisterHandler(L"execution.queue", L"enqueue",
        [this](const BridgeMessage& msg) -> CString { return HandleEnqueue(msg); });

    dispatcher.RegisterHandler(L"execution.queue", L"getQueue",
        [this](const BridgeMessage& msg) -> CString { return HandleGetQueue(msg); });

    dispatcher.RegisterHandler(L"execution.queue", L"cancelJob",
        [this](const BridgeMessage& msg) -> CString { return HandleCancelJob(msg); });

    dispatcher.RegisterHandler(L"execution.queue", L"cancelAll",
        [this](const BridgeMessage& msg) -> CString { return HandleCancelAll(msg); });

    dispatcher.RegisterHandler(L"execution.queue", L"retryJob",
        [this](const BridgeMessage& msg) -> CString { return HandleRetryJob(msg); });
}

CString JobQueueBridgeHandler::HandleEnqueue(const BridgeMessage& msg)
{
    CString strWorkflowId   = JsonExtractString(msg.m_strPayloadJson, L"workflowId");
    CString strWorkflowName = JsonExtractString(msg.m_strPayloadJson, L"workflowName");

    if (strWorkflowId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"workflowId is required\"}}";
    }

    CString strError;
    if (!m_service.EnqueueJob(strWorkflowId, strWorkflowName, m_hMainWnd, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"ENQUEUE_FAILED\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"queued\":true}}";
}

CString JobQueueBridgeHandler::HandleGetQueue(const BridgeMessage& msg)
{
    std::vector<ExecutionJob> arrJobs;
    m_service.GetQueue(arrJobs);

    CString strArray = L"[";
    for (int i = 0; i < (int)arrJobs.size(); ++i)
    {
        if (i > 0) strArray += L",";
        strArray += SerializeJob(arrJobs[i]);
    }
    strArray += L"]";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobs\":" + strArray + L"}}";
}

CString JobQueueBridgeHandler::HandleCancelJob(const BridgeMessage& msg)
{
    CString strJobId = JsonExtractString(msg.m_strPayloadJson, L"jobId");
    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    m_service.CancelJob(strJobId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + JsonEscapeString(strJobId) + L"\"}}";
}

BOOL JobQueueBridgeHandler::EnqueueWorkflow(const CString& strWorkflowId, const CString& strWorkflowName, HWND hMainWnd, CString& strError)
{
    return m_service.EnqueueJob(strWorkflowId, strWorkflowName, hMainWnd, strError);
}

void JobQueueBridgeHandler::CancelCurrentJob()
{
    m_service.CancelRunningJob();
}

const CString& JobQueueBridgeHandler::GetCurrentStepName() const
{
    return m_service.GetCurrentStepName();
}

const DataTable& JobQueueBridgeHandler::GetLastOutputTable() const
{
    return m_service.GetLastOutputTable();
}

CString JobQueueBridgeHandler::HandleCancelAll(const BridgeMessage& msg)
{
    std::vector<ExecutionJob> arrJobs;
    m_service.GetQueue(arrJobs);

    for (const ExecutionJob& job : arrJobs)
    {
        if (job.m_eStatus == JobStatus::Pending || job.m_eStatus == JobStatus::Running)
            m_service.CancelJob(job.m_strJobId);
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"cancelled\":true}}";
}

CString JobQueueBridgeHandler::HandleRetryJob(const BridgeMessage& msg)
{
    CString strJobId = JsonExtractString(msg.m_strPayloadJson, L"jobId");
    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    CString strError;
    if (!m_service.RetryJob(strJobId, m_hMainWnd, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"RETRY_FAILED\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + JsonEscapeString(strJobId) + L"\"}}";
}

CString JobQueueBridgeHandler::SerializeJob(const ExecutionJob& job) const
{
    CString strJson;
    strJson.Format(
        L"{"
        L"\"jobId\":\"%s\","
        L"\"workflowId\":\"%s\","
        L"\"workflowName\":\"%s\","
        L"\"status\":\"%s\","
        L"\"createdAt\":\"%s\","
        L"\"errorMessage\":\"%s\""
        L"}",
        (LPCWSTR)JsonEscapeString(job.m_strJobId),
        (LPCWSTR)JsonEscapeString(job.m_strWorkflowId),
        (LPCWSTR)JsonEscapeString(job.m_strWorkflowName),
        (LPCWSTR)JobStatusToString(job.m_eStatus),
        (LPCWSTR)JsonEscapeString(job.m_strCreatedAt),
        (LPCWSTR)JsonEscapeString(job.m_strErrorMessage)
    );
    return strJson;
}

CString JobQueueBridgeHandler::JobStatusToString(JobStatus eStatus) const
{
    switch (eStatus)
    {
    case JobStatus::Pending:   return L"pending";
    case JobStatus::Running:   return L"running";
    case JobStatus::Completed: return L"completed";
    case JobStatus::Failed:    return L"failed";
    case JobStatus::Cancelled: return L"cancelled";
    default:                   return L"unknown";
    }
}

