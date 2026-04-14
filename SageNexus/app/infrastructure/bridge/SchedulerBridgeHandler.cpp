#include "pch.h"
#include "app/infrastructure/bridge/SchedulerBridgeHandler.h"
#include "app/application/SageApp.h"
#include <string>
#include <vector>

SchedulerBridgeHandler::SchedulerBridgeHandler()
{
    m_service.LoadFromFile();
}

void SchedulerBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher)
{
    dispatcher.RegisterHandler(L"scheduler", L"getJobs",
        [this](const BridgeMessage& msg) -> CString { return HandleGetJobs(msg); });

    dispatcher.RegisterHandler(L"scheduler", L"addJob",
        [this](const BridgeMessage& msg) -> CString { return HandleAddJob(msg); });

    dispatcher.RegisterHandler(L"scheduler", L"removeJob",
        [this](const BridgeMessage& msg) -> CString { return HandleRemoveJob(msg); });

    dispatcher.RegisterHandler(L"scheduler", L"toggleJob",
        [this](const BridgeMessage& msg) -> CString { return HandleToggleJob(msg); });
}

void SchedulerBridgeHandler::GetDueJobs(std::vector<ScheduledJob>& arrDue)
{
    m_service.GetDueJobs(arrDue);
}

CString SchedulerBridgeHandler::HandleGetJobs(const BridgeMessage& msg)
{
    std::vector<ScheduledJob> arrJobs;
    m_service.GetJobs(arrJobs);

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

CString SchedulerBridgeHandler::HandleAddJob(const BridgeMessage& msg)
{
    CString strWorkflowId   = JsonExtractString(msg.m_strPayloadJson, L"workflowId");
    CString strWorkflowName = JsonExtractString(msg.m_strPayloadJson, L"workflowName");
    CString strTime         = JsonExtractString(msg.m_strPayloadJson, L"time");

    if (strWorkflowId.IsEmpty() || strTime.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"workflowId and time are required\"}}";
    }

    CString strJobId;
    m_service.AddJob(strWorkflowId, strWorkflowName, strTime, strJobId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + JsonEscapeString(strJobId) + L"\"}}";
}

CString SchedulerBridgeHandler::HandleRemoveJob(const BridgeMessage& msg)
{
    CString strJobId = JsonExtractString(msg.m_strPayloadJson, L"jobId");

    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    m_service.RemoveJob(strJobId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + JsonEscapeString(strJobId) + L"\"}}";
}

CString SchedulerBridgeHandler::HandleToggleJob(const BridgeMessage& msg)
{
    CString strJobId = JsonExtractString(msg.m_strPayloadJson, L"jobId");
    BOOL    bEnabled = JsonExtractBool(msg.m_strPayloadJson, L"enabled");

    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    m_service.ToggleJob(strJobId, bEnabled);

    CString strResult;
    strResult.Format(L"{\"jobId\":\"%s\",\"enabled\":%s}",
        (LPCWSTR)JsonEscapeString(strJobId),
        bEnabled ? L"true" : L"false");

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strResult + L"}";
}

CString SchedulerBridgeHandler::SerializeJob(const ScheduledJob& job) const
{
    CString strJson;
    strJson.Format(
        L"{"
        L"\"jobId\":\"%s\","
        L"\"workflowId\":\"%s\","
        L"\"workflowName\":\"%s\","
        L"\"time\":\"%s\","
        L"\"nextRunAt\":\"%s\","
        L"\"enabled\":%s,"
        L"\"createdAt\":\"%s\""
        L"}",
        (LPCWSTR)JsonEscapeString(job.m_strJobId),
        (LPCWSTR)JsonEscapeString(job.m_strWorkflowId),
        (LPCWSTR)JsonEscapeString(job.m_strWorkflowName),
        (LPCWSTR)JsonEscapeString(job.m_strTime),
        (LPCWSTR)JsonEscapeString(job.m_strNextRunAt),
        job.m_bEnabled ? L"true" : L"false",
        (LPCWSTR)JsonEscapeString(job.m_strCreatedAt)
    );
    return strJson;
}
