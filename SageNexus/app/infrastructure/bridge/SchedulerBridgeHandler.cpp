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
    CString strWorkflowId   = ExtractPayloadString(msg.m_strPayloadJson, L"workflowId");
    CString strWorkflowName = ExtractPayloadString(msg.m_strPayloadJson, L"workflowName");
    CString strTime         = ExtractPayloadString(msg.m_strPayloadJson, L"time");

    if (strWorkflowId.IsEmpty() || strTime.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"workflowId and time are required\"}}";
    }

    CString strJobId;
    m_service.AddJob(strWorkflowId, strWorkflowName, strTime, strJobId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + EscapeJson(strJobId) + L"\"}}";
}

CString SchedulerBridgeHandler::HandleRemoveJob(const BridgeMessage& msg)
{
    CString strJobId = ExtractPayloadString(msg.m_strPayloadJson, L"jobId");

    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    m_service.RemoveJob(strJobId);

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"jobId\":\"" + EscapeJson(strJobId) + L"\"}}";
}

CString SchedulerBridgeHandler::HandleToggleJob(const BridgeMessage& msg)
{
    CString strJobId = ExtractPayloadString(msg.m_strPayloadJson, L"jobId");
    BOOL    bEnabled = ExtractPayloadBool(msg.m_strPayloadJson, L"enabled");

    if (strJobId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"jobId is required\"}}";
    }

    m_service.ToggleJob(strJobId, bEnabled);

    CString strResult;
    strResult.Format(L"{\"jobId\":\"%s\",\"enabled\":%s}",
        (LPCWSTR)EscapeJson(strJobId),
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
        (LPCWSTR)EscapeJson(job.m_strJobId),
        (LPCWSTR)EscapeJson(job.m_strWorkflowId),
        (LPCWSTR)EscapeJson(job.m_strWorkflowName),
        (LPCWSTR)EscapeJson(job.m_strTime),
        (LPCWSTR)EscapeJson(job.m_strNextRunAt),
        job.m_bEnabled ? L"true" : L"false",
        (LPCWSTR)EscapeJson(job.m_strCreatedAt)
    );
    return strJson;
}

CString SchedulerBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return L"";

    size_t nQuoteOpen = json.find('"', nColon + 1);
    if (nQuoteOpen == std::string::npos) return L"";

    size_t nQuoteClose = json.find('"', nQuoteOpen + 1);
    if (nQuoteClose == std::string::npos) return L"";

    return Utf8ToWide(json.substr(nQuoteOpen + 1, nQuoteClose - nQuoteOpen - 1));
}

BOOL SchedulerBridgeHandler::ExtractPayloadBool(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return FALSE;

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return FALSE;

    size_t nStart = nColon + 1;
    while (nStart < json.size() && json[nStart] == ' ')
        ++nStart;

    return (json.substr(nStart, 4) == "true") ? TRUE : FALSE;
}

CString SchedulerBridgeHandler::EscapeJson(const CString& str) const
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        switch (ch)
        {
        case L'"':  strResult += L"\\\""; break;
        case L'\\': strResult += L"\\\\"; break;
        case L'\n': strResult += L"\\n";  break;
        case L'\r': strResult += L"\\r";  break;
        case L'\t': strResult += L"\\t";  break;
        default:    strResult += ch;      break;
        }
    }
    return strResult;
}
