#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/WorkflowService.h"
#include "app/domain/model/WorkflowTemplate.h"

class WorkflowBridgeHandler
{
public:
    WorkflowBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, HWND hMainWnd);
    const CString& GetCurrentStepName() const;

private:
    CString HandleGetWorkflows(const BridgeMessage& msg);
    CString HandleCreateWorkflow(const BridgeMessage& msg);
    CString HandleUpdateWorkflow(const BridgeMessage& msg);
    CString HandleDeleteWorkflow(const BridgeMessage& msg);
    CString HandleRunWorkflow(const BridgeMessage& msg);
    CString HandleCancelWorkflow(const BridgeMessage& msg);
    CString HandleGetTemplates(const BridgeMessage& msg);
    CString HandleCreateFromTemplate(const BridgeMessage& msg);

    CString SerializeWorkflow(const WorkflowDefinition& wf) const;
    CString SerializeTemplate(const WorkflowTemplate& tpl) const;
    CString SerializeStep(const WorkflowStep& step) const;
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString ExtractPayloadArray(const CString& strJson, const CString& strKey) const;
    std::vector<WorkflowStep> ParseStepsFromPayload(const CString& strStepsJson) const;
    WorkflowStep ParseStepFromPayload(const CString& strObj) const;
    CString EscapeJson(const CString& str) const;

    WorkflowService m_service;
    HWND            m_hMainWnd;
};
