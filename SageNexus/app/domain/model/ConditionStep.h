#pragma once
#include "pch.h"

struct ConditionStep
{
    CString m_strField;
    CString m_strOperator;
    CString m_strValue;
    CString m_strThenStepId;
    CString m_strElseStepId;
};
