#include "pch.h"
#include "app/application/services/TransformService.h"

BOOL TransformService::ApplySteps(DataTable& table, const std::vector<TransformStep>& arrSteps, CString& strError)
{
    for (const TransformStep& step : arrSteps)
    {
        BOOL bOk = FALSE;

        if (step.m_strType == L"trim")
            bOk = ApplyTrim(table, step, strError);
        else if (step.m_strType == L"renameColumn")
            bOk = ApplyRenameColumn(table, step, strError);
        else if (step.m_strType == L"replaceValue")
            bOk = ApplyReplaceValue(table, step, strError);
        else
        {
            strError.Format(L"알 수 없는 변환 유형입니다: %s", (LPCWSTR)step.m_strType);
            return FALSE;
        }

        if (!bOk)
            return FALSE;
    }
    return TRUE;
}

BOOL TransformService::ApplyTrim(DataTable& table, const TransformStep& step, CString& strError)
{
    if (step.m_strColumn.IsEmpty())
    {
        for (int i = 0; i < table.GetRowCount(); ++i)
        {
            DataRow& row = table.GetRowMutable(i);
            for (int j = 0; j < (int)row.m_arrCells.size(); ++j)
                row.m_arrCells[j].Trim();
        }
        return TRUE;
    }

    int nColIdx = table.FindColumnIndex(step.m_strColumn);
    if (nColIdx < 0)
    {
        strError.Format(L"컬럼을 찾을 수 없습니다: %s", (LPCWSTR)step.m_strColumn);
        return FALSE;
    }

    for (int i = 0; i < table.GetRowCount(); ++i)
    {
        DataRow& row = table.GetRowMutable(i);
        if (nColIdx < (int)row.m_arrCells.size())
            row.m_arrCells[nColIdx].Trim();
    }
    return TRUE;
}

BOOL TransformService::ApplyRenameColumn(DataTable& table, const TransformStep& step, CString& strError)
{
    int nColIdx = table.FindColumnIndex(step.m_strColumn);
    if (nColIdx < 0)
    {
        strError.Format(L"컬럼을 찾을 수 없습니다: %s", (LPCWSTR)step.m_strColumn);
        return FALSE;
    }

    DataColumn& col = table.GetColumnMutable(nColIdx);
    col.m_strDisplayNameKo = step.m_strParam1;
    return TRUE;
}

BOOL TransformService::ApplyReplaceValue(DataTable& table, const TransformStep& step, CString& strError)
{
    int nColIdx = table.FindColumnIndex(step.m_strColumn);
    if (nColIdx < 0)
    {
        strError.Format(L"컬럼을 찾을 수 없습니다: %s", (LPCWSTR)step.m_strColumn);
        return FALSE;
    }

    for (int i = 0; i < table.GetRowCount(); ++i)
    {
        DataRow& row = table.GetRowMutable(i);
        if (nColIdx < (int)row.m_arrCells.size())
        {
            if (row.m_arrCells[nColIdx] == step.m_strParam1)
                row.m_arrCells[nColIdx] = step.m_strParam2;
        }
    }
    return TRUE;
}
