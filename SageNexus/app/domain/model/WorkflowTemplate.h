#pragma once
#include "pch.h"
#include "app/domain/model/WorkflowStep.h"
#include <vector>

struct WorkflowTemplate
{
    CString                   m_strId;
    CString                   m_strName;
    CString                   m_strDescription;
    CString                   m_strCategory;
    std::vector<WorkflowStep> m_arrSteps;
};
