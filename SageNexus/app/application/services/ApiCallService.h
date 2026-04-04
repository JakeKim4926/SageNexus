#pragma once
#include "pch.h"
#include "app/domain/model/ApiCallAction.h"

class ApiCallService
{
public:
    BOOL CallApi(const ApiCallAction& action, CString& strError);
    BOOL CallApi(const ApiCallAction& action, CString& strResponseBody, CString& strError);

private:
    CString BuildPath(const wchar_t* szUrlPath, const wchar_t* szExtraInfo) const;
    void    ApplyHeaders(HINTERNET hRequest, const CString& strHeadersJson) const;
    BOOL    ReadResponseBody(HINTERNET hRequest, CString& strBody) const;
};
