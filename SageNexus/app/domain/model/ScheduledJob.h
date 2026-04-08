#pragma once
#include "pch.h"

struct ScheduledJob
{
    CString m_strJobId;
    CString m_strWorkflowId;
    CString m_strWorkflowName;
    CString m_strTime;
    CString m_strNextRunAt;
    BOOL    m_bEnabled;
    CString m_strCreatedAt;

    ScheduledJob()
        : m_bEnabled(TRUE)
    {}
};
