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
{
    m_connectorService.LoadFromFile();
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
    stepImport.m_strId         = L"";
    stepImport.m_strStepType   = STEP_TYPE_IMPORT;
    stepImport.m_strName       = L"CSV 불러오기";
    stepImport.m_strConfigJson = L"{\"filePath\":\"\"}";
    tplCsvXlsx.m_arrSteps.push_back(stepImport);

    WorkflowStep stepTransform;
    stepTransform.m_strId         = L"";
    stepTransform.m_strStepType   = STEP_TYPE_TRANSFORM;
    stepTransform.m_strName       = L"공백 제거";
    stepTransform.m_strConfigJson = L"{\"rules\":[{\"type\":\"trim\",\"column\":\"\",\"param1\":\"\",\"param2\":\"\"}]}";
    tplCsvXlsx.m_arrSteps.push_back(stepTransform);

    WorkflowStep stepExport;
    stepExport.m_strId         = L"";
    stepExport.m_strStepType   = STEP_TYPE_EXPORT;
    stepExport.m_strName       = L"XLSX 저장";
    stepExport.m_strConfigJson = L"{\"filePath\":\"\",\"format\":\"xlsx\"}";
    tplCsvXlsx.m_arrSteps.push_back(stepExport);

    arrTemplates.push_back(tplCsvXlsx);

    WorkflowTemplate tplWebHtml;
    tplWebHtml.m_strId          = L"tpl-web-to-html";
    tplWebHtml.m_strName        = L"웹 추출 → HTML 보고서";
    tplWebHtml.m_strDescription = L"웹 페이지에서 테이블을 추출해 HTML 보고서로 저장합니다.";
    tplWebHtml.m_strCategory    = L"webextract";

    WorkflowStep stepWeb;
    stepWeb.m_strId         = L"";
    stepWeb.m_strStepType   = STEP_TYPE_WEB_EXTRACT;
    stepWeb.m_strName       = L"웹 추출";
    stepWeb.m_strConfigJson = L"{\"url\":\"\",\"selector\":\"\"}";
    tplWebHtml.m_arrSteps.push_back(stepWeb);

    WorkflowStep stepHtml;
    stepHtml.m_strId         = L"";
    stepHtml.m_strStepType   = STEP_TYPE_EXPORT;
    stepHtml.m_strName       = L"HTML 보고서 저장";
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

BOOL WorkflowService::RunSync(
    const CString& strId,
    volatile BOOL& bCancelRef,
    HWND hNotifyWnd,
    CString& strError)
{
    WorkflowDefinition workflow;
    if (!GetWorkflow(strId, workflow, strError))
        return FALSE;

    if (workflow.m_arrSteps.empty())
    {
        strError = L"실행할 Step이 없습니다.";
        return FALSE;
    }

    m_bRunning    = TRUE;
    m_strLastError = L"";

    BOOL bSuccess = ExecuteWorkflowCore(workflow, bCancelRef, hNotifyWnd, strError);

    m_bRunning = FALSE;

    PostMessage(hNotifyWnd, WM_WORKFLOW_COMPLETE, (WPARAM)bSuccess, 0);
    return bSuccess;
}

const CString& WorkflowService::GetLastError() const
{
    return m_strLastError;
}

const CString& WorkflowService::GetCurrentStepName() const
{
    return m_strCurrentStepName;
}

const DataTable& WorkflowService::GetLastOutputTable() const
{
    return m_lastOutputTable;
}

// ============================================================
// 실제 Step 실행 로직 — RunSync에서만 호출한다.
// bCancelRef: 호출자(JobQueueService)가 관리하는 취소 플래그
// ============================================================
BOOL WorkflowService::ExecuteWorkflowCore(
    const WorkflowDefinition& workflow,
    volatile BOOL& bCancelRef,
    HWND hNotifyWnd,
    CString& strError)
{
    int  nTotal    = (int)workflow.m_arrSteps.size();
    BOOL bSuccess  = TRUE;
    DataTable currentTable;

    constexpr int MAX_STEP_ITERATIONS = 1000;
    int nIterations = 0;
    int i           = 0;

    while (i < nTotal && nIterations < MAX_STEP_ITERATIONS)
    {
        ++nIterations;

        if (bCancelRef)
        {
            strError  = L"사용자에 의해 취소되었습니다.";
            bSuccess  = FALSE;
            break;
        }

        const WorkflowStep& step    = workflow.m_arrSteps[i];
        int                 nNextIdx = i + 1;

        m_strCurrentStepName = step.m_strName.IsEmpty() ? step.m_strStepType : step.m_strName;
        PostMessage(hNotifyWnd, WM_WORKFLOW_PROGRESS, (WPARAM)(i + 1), (LPARAM)nTotal);

        CString strStepError;

        sageMgr.GetLogger().Log(LogLevel::Info,
            L"[Workflow] " + workflow.m_strName +
            L" step[" + step.m_strStepType + L"] " + step.m_strName);

        if (step.m_strStepType == STEP_TYPE_IMPORT)
        {
            CString strFilePath = JsonExtractString(step.m_strConfigJson, L"filePath");
            if (strFilePath.IsEmpty())
            {
                strError  = L"Import step: filePath가 비어 있습니다.";
                bSuccess  = FALSE;
                break;
            }
            ImportService svc;
            if (!svc.LoadFromFile(strFilePath, currentTable, strStepError))
            {
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_TRANSFORM)
        {
            std::vector<TransformStep> arrRules = ParseTransformSteps(step.m_strConfigJson);
            TransformService svc;
            if (!svc.ApplySteps(currentTable, arrRules, strStepError))
            {
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_EXPORT)
        {
            CString strFilePath = JsonExtractString(step.m_strConfigJson, L"filePath");
            CString strFormat   = JsonExtractString(step.m_strConfigJson, L"format");
            CString strLang     = JsonExtractString(step.m_strConfigJson, L"outputLanguage");

            if (strFilePath.IsEmpty())
            {
                strError  = L"Export step: filePath가 비어 있습니다.";
                bSuccess  = FALSE;
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
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_WEB_EXTRACT)
        {
            CString strUrl      = JsonExtractString(step.m_strConfigJson, L"url");
            CString strSelector = JsonExtractString(step.m_strConfigJson, L"selector");

            if (strUrl.IsEmpty())
            {
                strError  = L"WebExtract step: url이 비어 있습니다.";
                bSuccess  = FALSE;
                break;
            }
            WebExtractService svc;
            if (!svc.FetchAndExtract(strUrl, strSelector, currentTable, strStepError))
            {
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_SEND_EMAIL)
        {
            EmailAction action;
            action.m_strRecipients     = JsonExtractString(step.m_strConfigJson, L"recipients");
            action.m_strSubject        = JsonExtractString(step.m_strConfigJson, L"subject");
            action.m_strBody           = JsonExtractString(step.m_strConfigJson, L"body");
            action.m_strAttachFilePath = JsonExtractString(step.m_strConfigJson, L"attachFilePath");

            if (action.m_strRecipients.IsEmpty())
            {
                strError  = L"Send Email step: recipients가 비어 있습니다.";
                bSuccess  = FALSE;
                break;
            }
            EmailService svc;
            if (!svc.SendEmail(action, strStepError))
            {
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_CALL_API)
        {
            ApiCallAction action;
            CString strConnectorId = JsonExtractString(step.m_strConfigJson, L"connectorId");

            if (!strConnectorId.IsEmpty())
            {
                CString strUrlSuffix = JsonExtractString(step.m_strConfigJson, L"url");
                CString strMethod    = JsonExtractString(step.m_strConfigJson, L"method");
                CString strBody      = JsonExtractString(step.m_strConfigJson, L"body");
                CString strBuildErr;
                if (!m_connectorService.BuildAction(strConnectorId, strUrlSuffix, strMethod, strBody, action, strBuildErr))
                {
                    strError  = strBuildErr;
                    bSuccess  = FALSE;
                    break;
                }
            }
            else
            {
                action.m_strUrl         = JsonExtractString(step.m_strConfigJson, L"url");
                action.m_strMethod      = JsonExtractString(step.m_strConfigJson, L"method");
                action.m_strHeadersJson = JsonExtractString(step.m_strConfigJson, L"headers");
                action.m_strBody        = JsonExtractString(step.m_strConfigJson, L"body");
                CString strTimeout      = JsonExtractString(step.m_strConfigJson, L"timeout");
                if (!strTimeout.IsEmpty())
                    action.m_nTimeoutMs = _wtoi(strTimeout);

                if (action.m_strUrl.IsEmpty())
                {
                    strError  = L"Call API step: url이 비어 있습니다.";
                    bSuccess  = FALSE;
                    break;
                }
            }

            ApiCallService svc;
            if (!svc.CallApi(action, strStepError))
            {
                strError  = strStepError;
                bSuccess  = FALSE;
                break;
            }
        }
        else if (step.m_strStepType == STEP_TYPE_CONDITION)
        {
            ConditionStep cond     = ParseConditionStep(step.m_strConfigJson);
            BOOL          bMet     = EvaluateCondition(cond, currentTable);
            CString       strJumpId = bMet ? cond.m_strThenStepId : cond.m_strElseStepId;

            if (!strJumpId.IsEmpty())
            {
                int nJump = FindStepIndex(workflow.m_arrSteps, strJumpId);
                if (nJump < 0)
                {
                    strError  = L"Condition step: 연결된 StepId를 찾을 수 없습니다: " + strJumpId;
                    bSuccess  = FALSE;
                    break;
                }
                nNextIdx = nJump;
            }
        }

        if (!bSuccess) break;
        i = nNextIdx;
    }

    if (nIterations >= MAX_STEP_ITERATIONS)
    {
        strError  = L"Workflow 실행 한도를 초과했습니다. 조건 분기 무한 루프를 확인하세요.";
        bSuccess  = FALSE;
    }

    // 성공 시 최종 DataTable을 보존 — UI 스레드가 WM_WORKFLOW_COMPLETE 처리 후 읽는다.
    if (bSuccess)
        m_lastOutputTable = currentTable;

    m_strLastError = strError;

    ExecutionRecord record;
    record.m_strOperationType = L"workflow";
    record.m_strSourceName    = workflow.m_strName;
    record.m_bSuccess         = bSuccess;
    record.m_strErrorMessage  = strError;

    CString strSaveError;
    m_historyStore.SaveRecord(record, strSaveError);

    return bSuccess;
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
                step.m_strType   = JsonExtractString(strObj, L"type");
                step.m_strColumn = JsonExtractString(strObj, L"column");
                step.m_strParam1 = JsonExtractString(strObj, L"param1");
                step.m_strParam2 = JsonExtractString(strObj, L"param2");
                if (!step.m_strType.IsEmpty())
                    arrSteps.push_back(step);
                nObjStart = std::string::npos;
            }
        }
    }

    return arrSteps;
}

ConditionStep WorkflowService::ParseConditionStep(const CString& strConfigJson) const
{
    ConditionStep cond;
    cond.m_strField      = JsonExtractString(strConfigJson, L"field");
    cond.m_strOperator   = JsonExtractString(strConfigJson, L"operator");
    cond.m_strValue      = JsonExtractString(strConfigJson, L"value");
    cond.m_strThenStepId = JsonExtractString(strConfigJson, L"thenStepId");
    cond.m_strElseStepId = JsonExtractString(strConfigJson, L"elseStepId");
    return cond;
}

BOOL WorkflowService::EvaluateCondition(const ConditionStep& cond, const DataTable& table) const
{
    if (cond.m_strOperator == L"isEmpty")
        return table.IsEmpty() ? TRUE : FALSE;

    if (cond.m_strOperator == L"isNotEmpty")
        return !table.IsEmpty() ? TRUE : FALSE;

    if (table.GetRowCount() == 0)
        return FALSE;

    int nColIdx = table.FindColumnIndex(cond.m_strField);
    if (nColIdx < 0)
        return FALSE;

    const DataRow& row = table.GetRow(0);
    CString strCellValue;
    if (nColIdx < (int)row.m_arrCells.size())
        strCellValue = row.m_arrCells[nColIdx];

    if (cond.m_strOperator == L"equals")
        return strCellValue == cond.m_strValue ? TRUE : FALSE;

    if (cond.m_strOperator == L"notEquals")
        return strCellValue != cond.m_strValue ? TRUE : FALSE;

    if (cond.m_strOperator == L"contains")
        return strCellValue.Find(cond.m_strValue) >= 0 ? TRUE : FALSE;

    if (cond.m_strOperator == L"greaterThan")
        return _wtof(strCellValue) > _wtof(cond.m_strValue) ? TRUE : FALSE;

    if (cond.m_strOperator == L"lessThan")
        return _wtof(strCellValue) < _wtof(cond.m_strValue) ? TRUE : FALSE;

    return FALSE;
}

int WorkflowService::FindStepIndex(const std::vector<WorkflowStep>& arrSteps, const CString& strId) const
{
    for (int i = 0; i < (int)arrSteps.size(); ++i)
    {
        if (arrSteps[i].m_strId == strId)
            return i;
    }
    return -1;
}
