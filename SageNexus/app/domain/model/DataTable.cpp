#include "pch.h"
#include "DataTable.h"

DataTable::DataTable()
{
}

DataTable::DataTable(const CString& strSourceName)
    : m_strSourceName(strSourceName)
{
}

void DataTable::Clear()
{
    m_arrColumns.clear();
    m_arrRows.clear();
}

BOOL DataTable::IsEmpty() const
{
    return m_arrRows.empty() ? TRUE : FALSE;
}

void DataTable::AddColumn(const DataColumn& col)
{
    m_arrColumns.push_back(col);
}

void DataTable::AddRow(const DataRow& row)
{
    m_arrRows.push_back(row);
}

int DataTable::GetColumnCount() const
{
    return static_cast<int>(m_arrColumns.size());
}

int DataTable::GetRowCount() const
{
    return static_cast<int>(m_arrRows.size());
}

const DataColumn& DataTable::GetColumn(int nIndex) const
{
    return m_arrColumns[nIndex];
}

const DataRow& DataTable::GetRow(int nIndex) const
{
    return m_arrRows[nIndex];
}

DataColumn& DataTable::GetColumnMutable(int nIndex)
{
    return m_arrColumns[nIndex];
}

DataRow& DataTable::GetRowMutable(int nIndex)
{
    return m_arrRows[nIndex];
}

int DataTable::FindColumnIndex(const CString& strInternalName) const
{
    for (int i = 0; i < static_cast<int>(m_arrColumns.size()); ++i)
    {
        if (m_arrColumns[i].m_strInternalName == strInternalName)
            return i;
    }
    return -1;
}

const CString& DataTable::GetSourceName() const
{
    return m_strSourceName;
}

void DataTable::SetSourceName(const CString& strName)
{
    m_strSourceName = strName;
}
