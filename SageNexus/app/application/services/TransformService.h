#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"
#include "app/domain/model/TransformStep.h"

class TransformService
{
public:
    BOOL ApplySteps(DataTable& table, const std::vector<TransformStep>& arrSteps, CString& strError);

private:
    BOOL ApplyTrim(DataTable& table, const TransformStep& step, CString& strError);
    BOOL ApplyRenameColumn(DataTable& table, const TransformStep& step, CString& strError);
    BOOL ApplyReplaceValue(DataTable& table, const TransformStep& step, CString& strError);
};
