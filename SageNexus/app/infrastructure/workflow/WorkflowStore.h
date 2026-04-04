#pragma once
#include "pch.h"
#include "app/domain/model/WorkflowDefinition.h"
#include <vector>
#include <string>

class WorkflowStore
{
public:
    BOOL SaveWorkflow(WorkflowDefinition& workflow, CString& strError);
    BOOL LoadWorkflows(std::vector<WorkflowDefinition>& arrWorkflows, CString& strError) const;
    BOOL DeleteWorkflow(const CString& strId, CString& strError);

private:
    CString BuildFilePath() const;
    BOOL    EnsureDataDirectory(CString& strError) const;

    CString SerializeWorkflows(const std::vector<WorkflowDefinition>& arrWorkflows) const;
    CString SerializeWorkflow(const WorkflowDefinition& workflow) const;
    CString SerializeStep(const WorkflowStep& step) const;

    BOOL                 ParseWorkflows(const CString& strJson, std::vector<WorkflowDefinition>& arrWorkflows, CString& strError) const;
    WorkflowDefinition   ParseWorkflow(const CString& strObj) const;
    std::vector<WorkflowStep> ParseSteps(const CString& strStepsJson) const;
    WorkflowStep         ParseStep(const CString& strObj) const;

    CString              GenerateId() const;
    CString              GetCurrentTimestamp() const;

    CString              EscapeJsonString(const CString& str) const;
    CString              UnescapeJsonString(const CString& str) const;
    CString              ExtractStringField(const CString& strJson, const CString& strKey) const;
    CString              ExtractArrayField(const CString& strJson, const CString& strKey) const;
    std::vector<CString> SplitJsonObjects(const CString& strContent) const;

    static CString ReadFileAsWideString(const CString& strPath);
    static BOOL    WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError);
};
