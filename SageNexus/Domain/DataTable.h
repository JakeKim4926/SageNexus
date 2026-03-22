#pragma once
#include "pch.h"

// 컬럼 메타 정보
struct DataColumn
{
    CString m_strInternalName;   // 내부 식별자 (원본 또는 정규화된 이름)
    CString m_strSourceName;     // 원본 소스의 컬럼명
    CString m_strDisplayNameKo;  // 한국어 표시명
    CString m_strDisplayNameEn;  // 영어 표시명
    CString m_strDataType;       // "string" | "number" | "date" | "boolean"
};

// 행: 컬럼 인덱스에 대응하는 셀 값 배열
struct DataRow
{
    std::vector<CString> m_arrCells;
};

// 내부 표준 데이터 모델
// 입력(CSV/XLSX/웹추출 등)을 모두 이 구조로 수렴시킨다
class DataTable
{
public:
    DataTable();
    explicit DataTable(const CString& strSourceName);

    void Clear();
    BOOL IsEmpty() const;

    void AddColumn(const DataColumn& col);
    void AddRow(const DataRow& row);

    int GetColumnCount() const;
    int GetRowCount() const;

    const DataColumn& GetColumn(int nIndex) const;
    const DataRow&    GetRow(int nIndex) const;

    // 컬럼 인덱스 조회 (-1: 없음)
    int FindColumnIndex(const CString& strInternalName) const;

    const CString& GetSourceName() const;
    void SetSourceName(const CString& strName);

private:
    CString                  m_strSourceName;
    std::vector<DataColumn>  m_arrColumns;
    std::vector<DataRow>     m_arrRows;
};
