#pragma once
#include "pch.h"

struct WorkflowStep
{
    CString m_strId;
    CString m_strStepType;   // "import" | "transform" | "export" | "webExtract"
    CString m_strName;
    CString m_strConfigJson;
};
