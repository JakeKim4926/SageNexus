#include "pch.h"
#include "app/application/services/WorkflowService.h"
#include "app/application/services/ImportService.h"
#include "app/application/services/TransformService.h"
#include "app/application/services/ExportService.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <string>

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

BOOL WorkflowService::GetTemplates(std::vector<WorkflowTemplate>& arrTemplates, CString& strError)
{
    arrTemplates.clear();

    WorkflowTemplate tplCsvXlsx;
    tplCsvXlsx.m_strId          = L"tpl-csv-to-xlsx";
    tplCsvXlsx.m_strName        = L"CSV 정리 → XLSX 저장";
    tplCsvXlsx.m_strDescription = L"CSV 파일을 불러와 공백을 제거하고 XLSX로 저장합니다.";
    tplCsvXlsx.m_strCategory    = L"data";

    WorkflowStep stepImport;
    stepImport.m_strId       = L"";
    stepImport.m_strStepType = L"import";
    stepImport.m_strName     = L"CSV 불러오기";
    stepImport.m_strConfigJson = L"{\"filePath\":\"\"}";
    tplCsvXlsx.m_arrSteps.push_back(stepImport);

    WorkflowStep stepTransform;
    stepTransform.m_strId       = L"";
    stepTransform.m_strStepType = L"transform";
    stepTransform.m_strName     = L"공백 제거";
    stepTransform.m_strConfigJson = L"{\"rules\":[{\"type\":\"trim\",\"column\":\"\",\"param1\":\"\",\"param2\":\"\"}]}";
    tplCsvXlsx.m_arrSteps.push_back(stepTransform);

    WorkflowStep stepExport;
    stepExport.m_strId       = L"";
    stepExport.m_strStepType = L"export";
    stepExport.m_strName     = L"XLSX 저장";
    stepExport.m_strConfigJson = L"{\"filePath\":\"\",\"format\":\"xlsx\"}";
    tplCsvXlsx.m_arrSteps.push_back(stepExport);

    arrTemplates.push_back(tplCsvXlsx);

    WorkflowTemplate tplWebHtml;
    tplWebHtml.m_strId          = L"tpl-web-to-html";
    tplWebHtml.m_strName        = L"웹 추출 → HTML 보고서";
    tplWebHtml.m_strDescription = L"웹 페이지에서 테이블을 추출해 HTML 보고서로 저장합니다.";
    tplWebHtml.m_strCategory    = L"webextract";

    WorkflowStep stepWeb;
    stepWeb.m_strId       = L"";
    stepWeb.m_strStepType = L"webExtract";
    stepWeb.m_strName     = L"웹 추출";
    stepWeb.m_strConfigJson = L"{\"url\":\"\",\"selector\":\"\"}";
    tplWebHtml.m_arrSteps.push_back(stepWeb);

    WorkflowStep stepHtml;
    stepHtml.m_strId       = L"";
    stepHtml.m_strStepType = L"export";
    stepHtml.m_strName     = L"HTML 보고서 저장";
    stepHtml.m_strConfigJson = L"{\"filePath\":\"\",\"format\":\"html\"}";
    tplWebHtml.m_arrSteps.push_back(stepHtml);

    arrTemplates.push_back(tplWebHtml);

    return TRUE;
}

BOOL WorkflowService::CreateFromTemplate(const CString& strTemplateId, WorkflowDefinition& outWorkflow, CString& strError)
{
    std::vector<WorkflowTemplate> arrTemplates;
    if (!GetTemplates(arrTemplates, strError))
        return FALSE;

    const WorkflowTemplate* pTemplate = nullptr;
    for (const WorkflowTemplate& tpl : arrTemplates)
    {
        if (tpl.m_strId == strTemplateId)
        {
            pTemplate = &tpl;
            break;
        }
    }

    if (!pTemplate)
    {
        strError = L"템플릿을 찾을 수 없습니다: " + strTemplateId;
        return FALSE;
    }

    outWorkflow.m_strId          = L"";
    outWorkflow.m_strName        = pTemplate->m_strName;
    outWorkflow.m_strDescription = pTemplate->m_strDescription;
    outWorkflow.m_arrSteps       = pTemplate->m_arrSteps;

    if (!m_store.SaveWorkflow(outWorkflow, strError))
        return FALSE;

    return TRUE;
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

BOOL WorkflowService::RunSync(const CString& strId, volatile BOOL& bCancelRef, HWND hNotifyWnd, CString& strError)
{
    WorkflowDefinition workflow;
    if (!GetWorkflow(strId, workflow, strError))
        return FALSE;

    if (workflow.m_arrSteps.empty())
    {
        strError = L"실행할 Step이 없습니다.";
        return FALSE;
    }

    BOOL bPrevCancel    = m_bCancelRequested;
    m_bCancelRequested  = FALSE;
    m_bRunning          = TRUE;

    int   nTotal   = (int)workflow.m_arrSteps.size();
    BOOL  bSuccess = TRUE;
    DataTable currentTable;

    for (int i = 0; i < nTotal; ++i)
    {
        if (bCancelRef)
        {
            m_strLastError = L"사용자에 의해 취소되었습니다.";
            bSuccess       = FALSE;
            break;
        }

        const WorkflowStep& step = workflow.m_arrSteps[i];

        m_strCurrentStepName = step.m_strName.IsEmpty() ? step.m_strStepType : step.m_strName;
        PostMessage(hNotifyWnd, WM_WORKFLOW_PROGRESS, (WPARAM)(i + 1), (LPARAM)nTotal);

        CString strStepError;

        sageMgr.GetLogger().Log(LogLevel::Info,
            L"[JobQueue] " + workflow.m_strName +
            L" step[" + step.m_strStepType + L"] " + step.m_strName);

        if (step.m_strStepType == L"import")
        {
            CString strFilePath = ExtractConfigString(step.m_strConfigJson, L"filePath");
            if (strFilePath.IsEmpty())
            {
                m_strLastError = L"Import step: filePath가 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            ImportService svc;
            if (!svc.LoadFromFile(strFilePath, currentTable, strStepError))
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"transform")
        {
            std::vector<TransformStep> arrRules = ParseTransformSteps(step.m_strConfigJson);
            TransformService svc;
            if (!svc.ApplySteps(currentTable, arrRules, strStepError))
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"export")
        {
            CString strFilePath = ExtractConfigString(step.m_strConfigJson, L"filePath");
            CString strFormat   = ExtractConfigString(step.m_strConfigJson, L"format");
            CString strLang     = ExtractConfigString(step.m_strConfigJson, L"outputLanguage");
            if (strFilePath.IsEmpty())
            {
                m_strLastError = L"Export step: filePath가 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            if (strLang.IsEmpty())
                strLang = L"ko";

            ExportService svc;
            BOOL bExported = FALSE;
            if (strFormat == L"xlsx")
                bExported = svc.ExportToXlsx(currentTable, strFilePath, strLang, strStepError);
            else if (strFormat == L"html")
                bExported = svc.ExportToHtml(currentTable, strFilePath, strLang, strStepError);
            else
                bExported = svc.ExportToCsv(currentTable, strFilePath, strLang, strStepError);

            if (!bExported)
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"webExtract")
        {
            CString strUrl      = ExtractConfigString(step.m_strConfigJson, L"url");
            CString strSelector = ExtractConfigString(step.m_strConfigJson, L"selector");
            if (strUrl.IsEmpty())
            {
                m_strLastError = L"WebExtract step: url이 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            WebExtractService svc;
            if (!svc.FetchAndExtract(strUrl, strSelector, currentTable, strStepError))
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"sendEmail")
        {
            EmailAction action;
            action.m_strRecipients = ExtractConfigString(step.m_strConfigJson, L"to");
            action.m_strSubject    = ExtractConfigString(step.m_strConfigJson, L"subject");
            action.m_strBody       = ExtractConfigString(step.m_strConfigJson, L"body");
            EmailService svc;
            if (!svc.SendEmail(action, strStepError))
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"callApi")
        {
            ApiCallAction action;
            action.m_strUrl         = ExtractConfigString(step.m_strConfigJson, L"url");
            action.m_strMethod      = ExtractConfigString(step.m_strConfigJson, L"method");
            action.m_strHeadersJson = ExtractConfigString(step.m_strConfigJson, L"headers");
            action.m_strBody        = ExtractConfigString(step.m_strConfigJson, L"body");
            CString strTimeout      = ExtractConfigString(step.m_strConfigJson, L"timeout");
            action.m_nTimeoutMs     = strTimeout.IsEmpty() ? 30000 : _wtoi(strTimeout);
            if (action.m_strUrl.IsEmpty())
            {
                m_strLastError = L"Call API step: url이 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            ApiCallService svc;
            if (!svc.CallApi(action, strStepError))
            {
                m_strLastError = strStepError;
                bSuccess = FALSE;
                break;
            }
        }
    }

    ExecutionRecord record;
    record.m_strOperationType = L"workflow";
    record.m_strSourceName    = workflow.m_strName;
    record.m_bSuccess         = bSuccess;
    record.m_strErrorMessage  = m_strLastError;

    CString strSaveError;
    m_historyStore.SaveRecord(record, strSaveError);

    m_bRunning         = FALSE;
    m_bCancelRequested = bPrevCancel;

    if (!bSuccess)
        strError = m_strLastError;

    PostMessage(hNotifyWnd, WM_WORKFLOW_COMPLETE, (WPARAM)bSuccess, 0);
    return bSuccess;
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

const CString& WorkflowService::GetCurrentStepName() const
{
    return m_strCurrentStepName;
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

    int    nTotal    = (int)workflow.m_arrSteps.size();
    BOOL   bSuccess  = TRUE;
    DataTable currentTable;

    for (int i = 0; i < nTotal; ++i)
    {
        if (m_bCancelRequested)
        {
            m_strLastError = L"사용자에 의해 취소되었습니다.";
            bSuccess = FALSE;
            break;
        }

        const WorkflowStep& step = workflow.m_arrSteps[i];

        m_strCurrentStepName = step.m_strName.IsEmpty() ? step.m_strStepType : step.m_strName;
        PostMessage(hNotifyWnd, WM_WORKFLOW_PROGRESS, (WPARAM)(i + 1), (LPARAM)nTotal);

        CString strError;

        sageMgr.GetLogger().Log(LogLevel::Info,
            L"[Workflow] " + workflow.m_strName +
            L" step[" + step.m_strStepType + L"] " + step.m_strName);

        if (step.m_strStepType == L"import")
        {
            CString strFilePath = ExtractConfigString(step.m_strConfigJson, L"filePath");
            if (strFilePath.IsEmpty())
            {
                m_strLastError = L"Import step: filePath가 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            ImportService svc;
            if (!svc.LoadFromFile(strFilePath, currentTable, strError))
            {
                m_strLastError = strError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"transform")
        {
            std::vector<TransformStep> arrRules = ParseTransformSteps(step.m_strConfigJson);
            TransformService svc;
            if (!svc.ApplySteps(currentTable, arrRules, strError))
            {
                m_strLastError = strError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"webExtract")
        {
            CString strUrl      = ExtractConfigString(step.m_strConfigJson, L"url");
            CString strSelector = ExtractConfigString(step.m_strConfigJson, L"selector");

            if (strUrl.IsEmpty())
            {
                m_strLastError = L"WebExtract step: url이 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }

            WebExtractService svc;
            if (!svc.FetchAndExtract(strUrl, strSelector, currentTable, strError))
            {
                m_strLastError = strError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"export")
        {
            CString strFilePath = ExtractConfigString(step.m_strConfigJson, L"filePath");
            CString strFormat   = ExtractConfigString(step.m_strConfigJson, L"format");
            CString strLang     = ExtractConfigString(step.m_strConfigJson, L"outputLanguage");

            if (strFilePath.IsEmpty())
            {
                m_strLastError = L"Export step: filePath가 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }
            if (strLang.IsEmpty())
                strLang = L"ko";

            ExportService svc;
            if (strFormat == L"xlsx")
                bSuccess = svc.ExportToXlsx(currentTable, strFilePath, strLang, strError);
            else if (strFormat == L"html")
                bSuccess = svc.ExportToHtml(currentTable, strFilePath, strLang, strError);
            else
                bSuccess = svc.ExportToCsv(currentTable, strFilePath, strLang, strError);

            if (!bSuccess)
            {
                m_strLastError = strError;
                break;
            }
        }
        else if (step.m_strStepType == L"sendEmail")
        {
            EmailAction action;
            action.m_strRecipients     = ExtractConfigString(step.m_strConfigJson, L"recipients");
            action.m_strSubject        = ExtractConfigString(step.m_strConfigJson, L"subject");
            action.m_strBody           = ExtractConfigString(step.m_strConfigJson, L"body");
            action.m_strAttachFilePath = ExtractConfigString(step.m_strConfigJson, L"attachFilePath");

            if (action.m_strRecipients.IsEmpty())
            {
                m_strLastError = L"Send Email step: recipients가 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }

            EmailService svc;
            if (!svc.SendEmail(action, strError))
            {
                m_strLastError = strError;
                bSuccess = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == L"callApi")
        {
            ApiCallAction action;
            action.m_strUrl         = ExtractConfigString(step.m_strConfigJson, L"url");
            action.m_strMethod      = ExtractConfigString(step.m_strConfigJson, L"method");
            action.m_strHeadersJson = ExtractConfigString(step.m_strConfigJson, L"headers");
            action.m_strBody        = ExtractConfigString(step.m_strConfigJson, L"body");

            CString strTimeout = ExtractConfigString(step.m_strConfigJson, L"timeout");
            action.m_nTimeoutMs = strTimeout.IsEmpty() ? 30000 : _wtoi(strTimeout);

            if (action.m_strUrl.IsEmpty())
            {
                m_strLastError = L"Call API step: url이 비어 있습니다.";
                bSuccess = FALSE;
                break;
            }

            ApiCallService svc;
            if (!svc.CallApi(action, strError))
            {
                m_strLastError = strError;
                bSuccess = FALSE;
                break;
            }
        }
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

CString WorkflowService::ExtractConfigString(const CString& strConfigJson, const CString& strKey) const
{
    std::string json  = WideToUtf8(strConfigJson);
    std::string key   = WideToUtf8(strKey);
    std::string token = "\"" + key + "\"";

    size_t nKeyPos = json.find(token);
    if (nKeyPos == std::string::npos) return L"";

    size_t nColon = json.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos) return L"";

    size_t nQuoteOpen = json.find('"', nColon + 1);
    if (nQuoteOpen == std::string::npos) return L"";

    size_t nQuoteClose = nQuoteOpen + 1;
    while (nQuoteClose < json.size())
    {
        if (json[nQuoteClose] == '\\') { nQuoteClose += 2; continue; }
        if (json[nQuoteClose] == '"') break;
        ++nQuoteClose;
    }

    if (nQuoteClose >= json.size()) return L"";

    std::string val = json.substr(nQuoteOpen + 1, nQuoteClose - nQuoteOpen - 1);

    std::string unescaped;
    for (size_t i = 0; i < val.size(); ++i)
    {
        if (val[i] == '\\' && i + 1 < val.size())
        {
            ++i;
            switch (val[i])
            {
            case '"':  unescaped += '"';  break;
            case '\\': unescaped += '\\'; break;
            case '/':  unescaped += '/';  break;
            case 'n':  unescaped += '\n'; break;
            case 'r':  unescaped += '\r'; break;
            case 't':  unescaped += '\t'; break;
            default:   unescaped += val[i]; break;
            }
        }
        else
        {
            unescaped += val[i];
        }
    }

    return Utf8ToWide(unescaped);
}

std::vector<TransformStep> WorkflowService::ParseTransformSteps(const CString& strConfigJson) const
{
    std::vector<TransformStep> arrSteps;

    std::string json = WideToUtf8(strConfigJson);
    std::string rulesToken = "\"rules\"";
    size_t nPos = json.find(rulesToken);
    if (nPos == std::string::npos) return arrSteps;

    size_t nBracket = json.find('[', nPos);
    if (nBracket == std::string::npos) return arrSteps;

    int nDepth = 0;
    size_t nEnd = nBracket;
    for (; nEnd < json.size(); ++nEnd)
    {
        if (json[nEnd] == '[') ++nDepth;
        else if (json[nEnd] == ']') { --nDepth; if (nDepth == 0) break; }
    }

    std::string rulesJson = json.substr(nBracket, nEnd - nBracket + 1);

    nDepth = 0;
    size_t nObjStart = std::string::npos;
    for (size_t i = 0; i < rulesJson.size(); ++i)
    {
        if (rulesJson[i] == '"')
        {
            ++i;
            while (i < rulesJson.size())
            {
                if (rulesJson[i] == '\\') ++i;
                else if (rulesJson[i] == '"') break;
                ++i;
            }
            continue;
        }
        if (rulesJson[i] == '{') { if (nDepth == 0) nObjStart = i; ++nDepth; }
        else if (rulesJson[i] == '}')
        {
            --nDepth;
            if (nDepth == 0 && nObjStart != std::string::npos)
            {
                CString strObj = Utf8ToWide(rulesJson.substr(nObjStart, i - nObjStart + 1));
                TransformStep step;
                step.m_strType   = ExtractConfigString(strObj, L"type");
                step.m_strColumn = ExtractConfigString(strObj, L"column");
                step.m_strParam1 = ExtractConfigString(strObj, L"param1");
                step.m_strParam2 = ExtractConfigString(strObj, L"param2");
                if (!step.m_strType.IsEmpty())
                    arrSteps.push_back(step);
                nObjStart = std::string::npos;
            }
        }
    }

    return arrSteps;
}
