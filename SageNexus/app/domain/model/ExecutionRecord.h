#pragma once
#include "pch.h"

struct ExecutionRecord
{
    CString m_strRunId;
    CString m_strTimestamp;
    CString m_strOperationType;
    CString m_strSourceName;
    int     m_nRowCount;
    int     m_nColumnCount;
    CString m_strOutputPath;
    BOOL    m_bSuccess;
    CString m_strErrorMessage;

    ExecutionRecord()
        : m_nRowCount(0)
        , m_nColumnCount(0)
        , m_bSuccess(FALSE)
    {}
};
