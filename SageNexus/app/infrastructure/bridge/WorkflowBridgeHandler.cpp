#include "pch.h"
#include "app/infrastructure/bridge/WorkflowBridgeHandler.h"
#include "app/application/SageApp.h"
#include <string>

WorkflowBridgeHandler::WorkflowBridgeHandler()
    : m_hMainWnd(nullptr)
{
}

void WorkflowBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, HWND hMainWnd)
{
    m_hMainWnd = hMainWnd;

    dispatcher.RegisterHandler(L"workflow", L"getWorkflows",
        [this](const BridgeMessage& msg) -> CString { return HandleGetWorkflows(msg); });

    dispatcher.RegisterHandler(L"workflow", L"createWorkflow",
        [this](const BridgeMessage& msg) -> CString { return HandleCreateWorkflow(msg); });

    dispatcher.RegisterHandler(L"workflow", L"updateWorkflow",
        [this](const BridgeMessage& msg) -> CString { return HandleUpdateWorkflow(msg); });

    dispatcher.RegisterHandler(L"workflow", L"deleteWorkflow",
        [this](const BridgeMessage& msg) -> CString { return HandleDeleteWorkflow(msg); });

    dispatcher.RegisterHandler(L"workflow", L"runWorkflow",
        [this](const BridgeMessage& msg) -> CString { return HandleRunWorkflow(msg); });

    dispatcher.RegisterHandler(L"workflow", L"cancelWorkflow",
        [this](const BridgeMessage& msg) -> CString { return HandleCancelWorkflow(msg); });

    dispatcher.RegisterHandler(L"workflow.templates", L"getTemplates",
        [this](const BridgeMessage& msg) -> CString { return HandleGetTemplates(msg); });

    dispatcher.RegisterHandler(L"workflow.templates", L"createFromTemplate",
        [this](const BridgeMessage& msg) -> CString { return HandleCreateFromTemplate(msg); });
}

CString WorkflowBridgeHandler::HandleGetWorkflows(const BridgeMessage& msg)
{
    std::vector<WorkflowDefinition> arrWorkflows;
    CString strError;
    if (!m_service.GetWorkflows(arrWorkflows, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"LOAD_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    CString strArray = L"[";
    for (int i = 0; i < (int)arrWorkflows.size(); ++i)
    {
        if (i > 0) strArray += L",";
        strArray += SerializeWorkflow(arrWorkflows[i]);
    }
    strArray += L"]";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strArray + L"}";
}

CString WorkflowBridgeHandler::HandleCreateWorkflow(const BridgeMessage& msg)
{
    WorkflowDefinition wf;
    wf.m_strName        = ExtractPayloadString(msg.m_strPayloadJson, L"name");
    wf.m_strDescription = ExtractPayloadString(msg.m_strPayloadJson, L"description");

    if (wf.m_strName.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"name is required\"}}";
    }

    CString strError;
    if (!m_service.SaveWorkflow(wf, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"SAVE_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + SerializeWorkflow(wf) + L"}";
}

CString WorkflowBridgeHandler::HandleUpdateWorkflow(const BridgeMessage& msg)
{
    CString strId = ExtractPayloadString(msg.m_strPayloadJson, L"id");
    if (strId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"id is required\"}}";
    }

    WorkflowDefinition wf;
    CString strError;
    if (!m_service.GetWorkflow(strId, wf, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"NOT_FOUND\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    CString strName = ExtractPayloadString(msg.m_strPayloadJson, L"name");
    if (!strName.IsEmpty())
        wf.m_strName = strName;

    CString strDesc = ExtractPayloadString(msg.m_strPayloadJson, L"description");
    if (!strDesc.IsEmpty())
        wf.m_strDescription = strDesc;

    CString strStepsJson = ExtractPayloadArray(msg.m_strPayloadJson, L"steps");
    if (!strStepsJson.IsEmpty())
        wf.m_arrSteps = ParseStepsFromPayload(strStepsJson);

    if (!m_service.SaveWorkflow(wf, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"SAVE_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + SerializeWorkflow(wf) + L"}";
}

CString WorkflowBridgeHandler::HandleDeleteWorkflow(const BridgeMessage& msg)
{
    CString strId = ExtractPayloadString(msg.m_strPayloadJson, L"id");
    if (strId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"id is required\"}}";
    }

    CString strError;
    if (!m_service.DeleteWorkflow(strId, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"DELETE_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"id\":\"" + EscapeJson(strId) + L"\"}}";
}

CString WorkflowBridgeHandler::HandleRunWorkflow(const BridgeMessage& msg)
{
    CString strId = ExtractPayloadString(msg.m_strPayloadJson, L"id");
    if (strId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"id is required\"}}";
    }

    CString strError;
    if (!m_service.RunWorkflow(strId, m_hMainWnd, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"RUN_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"id\":\"" + EscapeJson(strId) + L"\",\"status\":\"running\"}}";
}

CString WorkflowBridgeHandler::HandleCancelWorkflow(const BridgeMessage& msg)
{
    m_service.CancelWorkflow();
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":{\"status\":\"cancelling\"}}";
}

CString WorkflowBridgeHandler::HandleGetTemplates(const BridgeMessage& msg)
{
    std::vector<WorkflowTemplate> arrTemplates;
    CString strError;
    if (!m_service.GetTemplates(arrTemplates, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"LOAD_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    CString strArray = L"[";
    for (int i = 0; i < (int)arrTemplates.size(); ++i)
    {
        if (i > 0) strArray += L",";
        strArray += SerializeTemplate(arrTemplates[i]);
    }
    strArray += L"]";

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strArray + L"}";
}

CString WorkflowBridgeHandler::HandleCreateFromTemplate(const BridgeMessage& msg)
{
    CString strTemplateId = ExtractPayloadString(msg.m_strPayloadJson, L"templateId");
    if (strTemplateId.IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"INVALID_PAYLOAD\",\"message\":\"templateId is required\"}}";
    }

    WorkflowDefinition wf;
    CString strError;
    if (!m_service.CreateFromTemplate(strTemplateId, wf, strError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"error\":{\"code\":\"CREATE_FAILED\",\"message\":\"" +
               EscapeJson(strError) + L"\"}}";
    }

    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + SerializeWorkflow(wf) + L"}";
}

const CString& WorkflowBridgeHandler::GetCurrentStepName() const
{
    return m_service.GetCurrentStepName();
}

CString WorkflowBridgeHandler::SerializeWorkflow(const WorkflowDefinition& wf) const
{
    CString strSteps = L"[";
    for (int i = 0; i < (int)wf.m_arrSteps.size(); ++i)
    {
        if (i > 0) strSteps += L",";
        strSteps += SerializeStep(wf.m_arrSteps[i]);
    }
    strSteps += L"]";

    CString strJson;
    strJson.Format(
        L"{"
        L"\"id\":\"%s\","
        L"\"name\":\"%s\","
        L"\"description\":\"%s\","
        L"\"createdAt\":\"%s\","
        L"\"updatedAt\":\"%s\","
        L"\"steps\":%s"
        L"}",
        (LPCWSTR)EscapeJson(wf.m_strId),
        (LPCWSTR)EscapeJson(wf.m_strName),
        (LPCWSTR)EscapeJson(wf.m_strDescription),
        (LPCWSTR)EscapeJson(wf.m_strCreatedAt),
        (LPCWSTR)EscapeJson(wf.m_strUpdatedAt),
        (LPCWSTR)strSteps
    );
    return strJson;
}

CString WorkflowBridgeHandler::SerializeTemplate(const WorkflowTemplate& tpl) const
{
    CString strSteps = L"[";
    for (int i = 0; i < (int)tpl.m_arrSteps.size(); ++i)
    {
        if (i > 0) strSteps += L",";
        strSteps += SerializeStep(tpl.m_arrSteps[i]);
    }
    strSteps += L"]";

    CString strJson;
    strJson.Format(
        L"{"
        L"\"id\":\"%s\","
        L"\"name\":\"%s\","
        L"\"description\":\"%s\","
        L"\"category\":\"%s\","
        L"\"steps\":%s"
        L"}",
        (LPCWSTR)EscapeJson(tpl.m_strId),
        (LPCWSTR)EscapeJson(tpl.m_strName),
        (LPCWSTR)EscapeJson(tpl.m_strDescription),
        (LPCWSTR)EscapeJson(tpl.m_strCategory),
        (LPCWSTR)strSteps
    );
    return strJson;
}

CString WorkflowBridgeHandler::SerializeStep(const WorkflowStep& step) const
{
    CString strJson;
    strJson.Format(
        L"{"
        L"\"id\":\"%s\","
        L"\"stepType\":\"%s\","
        L"\"name\":\"%s\","
        L"\"configJson\":\"%s\""
        L"}",
        (LPCWSTR)EscapeJson(step.m_strId),
        (LPCWSTR)EscapeJson(step.m_strStepType),
        (LPCWSTR)EscapeJson(step.m_strName),
        (LPCWSTR)EscapeJson(step.m_strConfigJson)
    );
    return strJson;
}

CString WorkflowBridgeHandler::ExtractPayloadString(const CString& strJson, const CString& strKey) const
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

CString WorkflowBridgeHandler::ExtractPayloadArray(const CString& strJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return L"";

    size_t nBracket = nColon + 1;
    while (nBracket < json.size() && json[nBracket] == ' ')
        ++nBracket;

    if (nBracket >= json.size() || json[nBracket] != '[')
        return L"";

    int nDepth = 0;
    size_t nEnd = nBracket;
    for (; nEnd < json.size(); ++nEnd)
    {
        if (json[nEnd] == '[') ++nDepth;
        else if (json[nEnd] == ']')
        {
            --nDepth;
            if (nDepth == 0) break;
        }
    }

    return Utf8ToWide(json.substr(nBracket, nEnd - nBracket + 1));
}

std::vector<WorkflowStep> WorkflowBridgeHandler::ParseStepsFromPayload(const CString& strStepsJson) const
{
    std::vector<WorkflowStep> arrSteps;
    std::string json = WideToUtf8(strStepsJson);

    int nDepth = 0;
    size_t nStart = std::string::npos;

    for (size_t i = 0; i < json.size(); ++i)
    {
        if (json[i] == '"')
        {
            ++i;
            while (i < json.size())
            {
                if (json[i] == '\\') ++i;
                else if (json[i] == '"') break;
                ++i;
            }
            continue;
        }
        if (json[i] == '{')
        {
            if (nDepth == 0) nStart = i;
            ++nDepth;
        }
        else if (json[i] == '}')
        {
            --nDepth;
            if (nDepth == 0 && nStart != std::string::npos)
            {
                CString strObj = Utf8ToWide(json.substr(nStart, i - nStart + 1));
                arrSteps.push_back(ParseStepFromPayload(strObj));
                nStart = std::string::npos;
            }
        }
    }
    return arrSteps;
}

WorkflowStep WorkflowBridgeHandler::ParseStepFromPayload(const CString& strObj) const
{
    WorkflowStep step;
    step.m_strId         = ExtractPayloadString(strObj, L"id");
    step.m_strStepType   = ExtractPayloadString(strObj, L"stepType");
    step.m_strName       = ExtractPayloadString(strObj, L"name");
    step.m_strConfigJson = ExtractPayloadString(strObj, L"configJson");
    return step;
}

CString WorkflowBridgeHandler::EscapeJson(const CString& str) const
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
