#pragma once
#include "pch.h"
#include "app/domain/model/WorkflowStep.h"
#include <vector>

struct WorkflowDefinition
{
    CString                    m_strId;
    CString                    m_strName;
    CString                    m_strDescription;
    CString                    m_strCreatedAt;
    CString                    m_strUpdatedAt;
    std::vector<WorkflowStep>  m_arrSteps;
};
