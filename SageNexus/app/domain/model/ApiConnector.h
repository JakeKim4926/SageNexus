#pragma once
#include "pch.h"

struct ApiConnector
{
    CString m_strConnectorId;
    CString m_strName;
    CString m_strBaseUrl;
    CString m_strHeadersJson;
    CString m_strAuthType;    // "none" | "bearer" | "apikey"
    CString m_strAuthValue;
    BOOL    m_bEnabled;
    CString m_strCreatedAt;

    ApiConnector()
        : m_bEnabled(TRUE)
    {}
};
