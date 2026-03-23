#include "pch.h"
#include "app/infrastructure/writers/CsvWriter.h"

BOOL CsvWriter::Write(const DataTable& table, const CString& strFilePath, CString& strError)
{
    std::wstring content;
    content.reserve(256 * 1024);

    for (int i = 0; i < table.GetColumnCount(); ++i)
    {
        if (i > 0) content += L",";
        const DataColumn& col = table.GetColumn(i);
        CString strHeader = col.m_strDisplayNameKo.IsEmpty() ? col.m_strInternalName : col.m_strDisplayNameKo;
        content += (LPCWSTR)QuoteField(strHeader);
    }
    content += L"\r\n";

    for (int i = 0; i < table.GetRowCount(); ++i)
    {
        const DataRow& row = table.GetRow(i);
        for (int j = 0; j < (int)row.m_arrCells.size(); ++j)
        {
            if (j > 0) content += L",";
            content += (LPCWSTR)QuoteField(row.m_arrCells[j]);
        }
        content += L"\r\n";
    }

    int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (nUtf8Len <= 0)
    {
        strError = L"UTF-8 변환에 실패했습니다.";
        return FALSE;
    }

    std::vector<char> arrUtf8(nUtf8Len);
    WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, arrUtf8.data(), nUtf8Len, nullptr, nullptr);

    HANDLE hFile = CreateFileW(strFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        strError.Format(L"파일을 열 수 없습니다: %s", (LPCWSTR)strFilePath);
        return FALSE;
    }

    const char arrBom[3] = { '\xEF', '\xBB', '\xBF' };
    DWORD dwWritten = 0;
    WriteFile(hFile, arrBom, 3, &dwWritten, nullptr);

    int nDataLen = nUtf8Len - 1;
    WriteFile(hFile, arrUtf8.data(), static_cast<DWORD>(nDataLen), &dwWritten, nullptr);

    CloseHandle(hFile);
    return TRUE;
}

CString CsvWriter::QuoteField(const CString& str) const
{
    if (!NeedsQuoting(str))
        return str;

    CString strResult = L"\"";
    for (int i = 0; i < str.GetLength(); ++i)
    {
        if (str[i] == L'"')
            strResult += L"\"\"";
        else
            strResult += str[i];
    }
    strResult += L"\"";
    return strResult;
}

BOOL CsvWriter::NeedsQuoting(const CString& str) const
{
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        if (ch == L',' || ch == L'"' || ch == L'\n' || ch == L'\r')
            return TRUE;
    }
    return FALSE;
}
