#pragma once
#include "pch.h"
#include "app/domain/model/EmailAction.h"

class EmailService
{
public:
    BOOL SendEmail(const EmailAction& action, CString& strError);

private:
    std::vector<CString> ParseRecipients(const CString& strRecipients) const;
};
