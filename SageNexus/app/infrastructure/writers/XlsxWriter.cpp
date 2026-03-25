#include "pch.h"
#include "app/infrastructure/writers/XlsxWriter.h"

BOOL XlsxWriter::Write(const DataTable& table, const CString& strFilePath, CString& strError)
{
    wchar_t szTempBase[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, szTempBase);

    wchar_t szTempName[MAX_PATH] = {};
    GetTempFileNameW(szTempBase, L"snx", 0, szTempName);
    DeleteFileW(szTempName);
    CreateDirectoryW(szTempName, nullptr);
    CString strWorkDir = szTempName;

    CString strSrcDir = strWorkDir + L"\\src";
    CreateDirectoryW(strSrcDir, nullptr);

    if (!WriteXmlFiles(strSrcDir, table, strError))
    {
        CleanupDir(strWorkDir);
        return FALSE;
    }

    CString strTempXlsx = strWorkDir + L"\\output.xlsx";
    if (!CompressToXlsx(strSrcDir, strTempXlsx, strError))
    {
        CleanupDir(strWorkDir);
        return FALSE;
    }

    if (!CopyFileW(strTempXlsx, strFilePath, FALSE))
    {
        strError.Format(L"파일을 저장할 수 없습니다: %s", (LPCWSTR)strFilePath);
        CleanupDir(strWorkDir);
        return FALSE;
    }

    CleanupDir(strWorkDir);
    return TRUE;
}

BOOL XlsxWriter::WriteXmlFiles(const CString& strSrcDir, const DataTable& table, CString& strError) const
{
    CreateDirectoryW(strSrcDir + L"\\_rels", nullptr);
    CreateDirectoryW(strSrcDir + L"\\xl", nullptr);
    CreateDirectoryW(strSrcDir + L"\\xl\\_rels", nullptr);
    CreateDirectoryW(strSrcDir + L"\\xl\\worksheets", nullptr);

    if (!WriteTextFile(strSrcDir + L"\\[Content_Types].xml", BuildContentTypes(), strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\_rels\\.rels",        BuildRootRels(),     strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\xl\\workbook.xml",    BuildWorkbook(),     strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\xl\\_rels\\workbook.xml.rels", BuildWorkbookRels(), strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\xl\\styles.xml",      BuildStyles(),       strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\xl\\worksheets\\sheet1.xml", BuildSheet(table), strError)) return FALSE;

    return TRUE;
}

BOOL XlsxWriter::WriteTextFile(const CString& strPath, const std::wstring& content, CString& strError) const
{
    int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (nUtf8Len <= 0)
    {
        strError = L"UTF-8 변환에 실패했습니다.";
        return FALSE;
    }

    std::vector<char> arrUtf8(nUtf8Len);
    WideCharToMultiByte(CP_UTF8, 0, content.c_str(), -1, arrUtf8.data(), nUtf8Len, nullptr, nullptr);

    HANDLE hFile = CreateFileW(strPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        strError.Format(L"파일을 생성할 수 없습니다: %s", (LPCWSTR)strPath);
        return FALSE;
    }

    DWORD dwWritten = 0;
    int nDataLen = nUtf8Len - 1;
    WriteFile(hFile, arrUtf8.data(), static_cast<DWORD>(nDataLen), &dwWritten, nullptr);
    CloseHandle(hFile);
    return TRUE;
}

BOOL XlsxWriter::CompressToXlsx(const CString& strSrcDir, const CString& strDestPath, CString& strError) const
{
    CString strSrcEsc  = strSrcDir;
    strSrcEsc.Replace(L"'", L"''");
    CString strDestEsc = strDestPath;
    strDestEsc.Replace(L"'", L"''");

    CString strCmd;
    strCmd.Format(
        L"powershell.exe -NoProfile -NonInteractive -Command \""
        L"Add-Type -Assembly System.IO.Compression.FileSystem; "
        L"[System.IO.Compression.ZipFile]::CreateFromDirectory('%s', '%s')\"",
        (LPCWSTR)strSrcEsc, (LPCWSTR)strDestEsc);

    std::vector<wchar_t> arrCmd(strCmd.GetLength() + 1, L'\0');
    wmemcpy(arrCmd.data(), strCmd.GetString(), strCmd.GetLength());

    STARTUPINFOW si = {};
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(nullptr, arrCmd.data(), nullptr, nullptr,
                        FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        strError = L"PowerShell 실행에 실패했습니다.";
        return FALSE;
    }

    WaitForSingleObject(pi.hProcess, 60000);

    DWORD dwExit = 0;
    GetExitCodeProcess(pi.hProcess, &dwExit);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (dwExit != 0)
    {
        strError = L"XLSX 압축 생성에 실패했습니다.";
        return FALSE;
    }

    return TRUE;
}

void XlsxWriter::CleanupDir(const CString& strDir) const
{
    CString strPattern = strDir + L"\\*";
    WIN32_FIND_DATAW fd = {};
    HANDLE hFind = FindFirstFileW(strPattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        RemoveDirectoryW(strDir);
        return;
    }

    do
    {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;

        CString strChild = strDir + L"\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            CleanupDir(strChild);
        else
            DeleteFileW(strChild);
    }
    while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    RemoveDirectoryW(strDir);
}

std::wstring XlsxWriter::BuildContentTypes() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           L"<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
           L"<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
           L"<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
           L"<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
           L"<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
           L"</Types>";
}

std::wstring XlsxWriter::BuildRootRels() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           L"<Relationship Id=\"rId1\" "
           L"Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" "
           L"Target=\"xl/workbook.xml\"/>"
           L"</Relationships>";
}

std::wstring XlsxWriter::BuildWorkbook() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
           L"xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
           L"<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
           L"</workbook>";
}

std::wstring XlsxWriter::BuildWorkbookRels() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           L"<Relationship Id=\"rId1\" "
           L"Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" "
           L"Target=\"worksheets/sheet1.xml\"/>"
           L"<Relationship Id=\"rId2\" "
           L"Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" "
           L"Target=\"styles.xml\"/>"
           L"</Relationships>";
}

std::wstring XlsxWriter::BuildStyles() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
           L"<fonts count=\"2\">"
           L"<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
           L"<font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font>"
           L"</fonts>"
           L"<fills count=\"2\">"
           L"<fill><patternFill patternType=\"none\"/></fill>"
           L"<fill><patternFill patternType=\"gray125\"/></fill>"
           L"</fills>"
           L"<borders count=\"1\">"
           L"<border><left/><right/><top/><bottom/><diagonal/></border>"
           L"</borders>"
           L"<cellStyleXfs count=\"1\">"
           L"<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/>"
           L"</cellStyleXfs>"
           L"<cellXfs count=\"2\">"
           L"<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
           L"<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>"
           L"</cellXfs>"
           L"</styleSheet>";
}

std::wstring XlsxWriter::BuildSheet(const DataTable& table) const
{
    std::wstring xml;
    xml.reserve(1024 * 1024);

    xml += L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
    xml += L"<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
    xml += L"<sheetData>";

    xml += L"<row r=\"1\">";
    for (int i = 0; i < table.GetColumnCount(); ++i)
    {
        const DataColumn& col = table.GetColumn(i);
        CString strHeader = col.m_strDisplayNameKo.IsEmpty() ? col.m_strInternalName : col.m_strDisplayNameKo;
        xml += L"<c r=\"";
        xml += (LPCWSTR)(ColIndexToRef(i) + L"1");
        xml += L"\" t=\"inlineStr\" s=\"1\"><is><t>";
        xml += EscapeXml(strHeader);
        xml += L"</t></is></c>";
    }
    xml += L"</row>";

    for (int r = 0; r < table.GetRowCount(); ++r)
    {
        const DataRow& row = table.GetRow(r);
        wchar_t szRowNum[16] = {};
        _itow_s(r + 2, szRowNum, 16, 10);

        xml += L"<row r=\"";
        xml += szRowNum;
        xml += L"\">";

        int nCellCount = __min((int)row.m_arrCells.size(), table.GetColumnCount());
        for (int c = 0; c < nCellCount; ++c)
        {
            xml += L"<c r=\"";
            xml += (LPCWSTR)(ColIndexToRef(c) + CString(szRowNum));
            xml += L"\" t=\"inlineStr\"><is><t>";
            xml += EscapeXml(row.m_arrCells[c]);
            xml += L"</t></is></c>";
        }
        xml += L"</row>";
    }

    xml += L"</sheetData></worksheet>";
    return xml;
}

CString XlsxWriter::ColIndexToRef(int nColIdx) const
{
    CString strRef;
    int n = nColIdx + 1;
    while (n > 0)
    {
        n--;
        strRef = CString(wchar_t(L'A' + n % 26)) + strRef;
        n /= 26;
    }
    return strRef;
}

std::wstring XlsxWriter::EscapeXml(const CString& str) const
{
    std::wstring result;
    result.reserve(str.GetLength());
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        switch (ch)
        {
        case L'&':  result += L"&amp;";  break;
        case L'<':  result += L"&lt;";   break;
        case L'>':  result += L"&gt;";   break;
        case L'"':  result += L"&quot;"; break;
        case L'\'': result += L"&apos;"; break;
        default:    result += ch;        break;
        }
    }
    return result;
}
