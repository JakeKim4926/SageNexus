#pragma once
#include "pch.h"
#include "EnumDefine.h"

struct ExecutionJob
{
    CString   m_strJobId;
    CString   m_strWorkflowId;
    CString   m_strWorkflowName;
    JobStatus m_eStatus;
    CString   m_strCreatedAt;
    CString   m_strErrorMessage;

    ExecutionJob()
        : m_eStatus(JobStatus::Pending)
    {}
};
