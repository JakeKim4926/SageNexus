#pragma once
#include "pch.h"

struct ApiCallAction
{
    CString m_strUrl;
    CString m_strMethod;
    CString m_strHeadersJson;
    CString m_strBody;
    int     m_nTimeoutMs;
};
