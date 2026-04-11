#pragma once
#include "pch.h"
#include "app/domain/model/WorkflowDefinition.h"
#include "app/domain/model/WorkflowTemplate.h"
#include "app/domain/model/TransformStep.h"
#include "app/domain/model/ConditionStep.h"
#include "app/domain/model/DataTable.h"
#include "app/infrastructure/workflow/WorkflowStore.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "app/application/services/WebExtractService.h"
#include "app/application/services/EmailService.h"
#include "app/application/services/ApiCallService.h"
#include "app/application/services/ApiConnectorService.h"
#include <vector>

class WorkflowService
{
public:
    WorkflowService();

    BOOL GetWorkflows(std::vector<WorkflowDefinition>& arrWorkflows, CString& strError);
    BOOL GetWorkflow(const CString& strId, WorkflowDefinition& outWorkflow, CString& strError);
    BOOL SaveWorkflow(WorkflowDefinition& workflow, CString& strError);
    BOOL DeleteWorkflow(const CString& strId, CString& strError);

    BOOL GetTemplates(std::vector<WorkflowTemplate>& arrTemplates, CString& strError);
    BOOL CreateFromTemplate(const CString& strTemplateId, WorkflowDefinition& outWorkflow, CString& strError);

    BOOL RunWorkflow(const CString& strId, HWND hNotifyWnd, CString& strError);
    BOOL RunSync(const CString& strId, volatile BOOL& bCancelRef, HWND hNotifyWnd, CString& strError);
    void CancelWorkflow();

    BOOL           IsRunning() const;
    const CString& GetLastError() const;
    const CString& GetCurrentStepName() const;

private:
    struct RunContext
    {
        WorkflowService*   pService;
        WorkflowDefinition workflow;
        HWND               hNotifyWnd;
    };

    static DWORD WINAPI RunThread(LPVOID pParam);
    void ExecuteSteps(const WorkflowDefinition& workflow, HWND hNotifyWnd);

    CString                        ExtractConfigString(const CString& strConfigJson, const CString& strKey) const;
    std::vector<TransformStep>     ParseTransformSteps(const CString& strConfigJson) const;
    ConditionStep                  ParseConditionStep(const CString& strConfigJson) const;
    BOOL                           EvaluateCondition(const ConditionStep& cond, const DataTable& table) const;
    int                            FindStepIndex(const std::vector<WorkflowStep>& arrSteps, const CString& strId) const;

    WorkflowStore          m_store;
    ExecutionHistoryStore  m_historyStore;
    ApiConnectorService    m_connectorService;
    BOOL                   m_bRunning;
    volatile BOOL          m_bCancelRequested;
    CString                m_strLastError;
    CString                m_strCurrentStepName;
};
