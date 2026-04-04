#include "pch.h"
#include "app/infrastructure/exporters/WordExporter.h"
#include <string>
#include <vector>
#include <sstream>

BOOL WordExporter::Export(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
{
    wchar_t szTempBase[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, szTempBase);

    wchar_t szTempName[MAX_PATH] = {};
    GetTempFileNameW(szTempBase, L"snw", 0, szTempName);
    DeleteFileW(szTempName);
    CreateDirectoryW(szTempName, nullptr);
    CString strWorkDir = szTempName;

    CString strSrcDir = strWorkDir + L"\\src";
    CreateDirectoryW(strSrcDir, nullptr);

    if (!WriteDocxFiles(strSrcDir, table, strLang, strError))
    {
        CleanupDir(strWorkDir);
        return FALSE;
    }

    CString strTempDocx = strWorkDir + L"\\output.docx";
    if (!CompressToDocx(strSrcDir, strTempDocx, strError))
    {
        CleanupDir(strWorkDir);
        return FALSE;
    }

    if (!CopyFileW(strTempDocx, strFilePath, FALSE))
    {
        strError.Format(L"파일을 저장할 수 없습니다: %s", (LPCWSTR)strFilePath);
        CleanupDir(strWorkDir);
        return FALSE;
    }

    CleanupDir(strWorkDir);
    return TRUE;
}

BOOL WordExporter::WriteDocxFiles(const CString& strSrcDir, const DataTable& table, const CString& strLang, CString& strError) const
{
    CreateDirectoryW(strSrcDir + L"\\_rels", nullptr);
    CreateDirectoryW(strSrcDir + L"\\word", nullptr);
    CreateDirectoryW(strSrcDir + L"\\word\\_rels", nullptr);

    if (!WriteTextFile(strSrcDir + L"\\[Content_Types].xml", BuildContentTypes(), strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\_rels\\.rels",        BuildRootRels(),     strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\word\\document.xml",  BuildDocument(table, strLang), strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\word\\_rels\\document.xml.rels", BuildDocumentRels(), strError)) return FALSE;
    if (!WriteTextFile(strSrcDir + L"\\word\\styles.xml",    BuildStyles(),       strError)) return FALSE;

    return TRUE;
}

BOOL WordExporter::WriteTextFile(const CString& strPath, const std::wstring& content, CString& strError) const
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

BOOL WordExporter::CompressToDocx(const CString& strSrcDir, const CString& strDestPath, CString& strError) const
{
    std::wstring wSrc  = (LPCWSTR)strSrcDir;
    std::wstring wDest = (LPCWSTR)strDestPath;

    std::wstring wScript =
        L"Add-Type -AssemblyName System.IO.Compression.FileSystem;"
        L"[System.IO.Compression.ZipFile]::CreateFromDirectory('" + wSrc + L"','" + wDest + L"');";

    CString strCmd;
    strCmd.Format(L"powershell -NoProfile -NonInteractive -Command \"%s\"", wScript.c_str());

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(nullptr, strCmd.GetBuffer(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        strCmd.ReleaseBuffer();
        strError = L"PowerShell 프로세스를 시작할 수 없습니다.";
        return FALSE;
    }
    strCmd.ReleaseBuffer();

    WaitForSingleObject(pi.hProcess, 30000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (GetFileAttributesW(strDestPath) == INVALID_FILE_ATTRIBUTES)
    {
        strError = L"docx 압축 파일 생성에 실패했습니다.";
        return FALSE;
    }

    return TRUE;
}

void WordExporter::CleanupDir(const CString& strDir) const
{
    CString strCmd;
    strCmd.Format(L"cmd /c rmdir /s /q \"%s\"", (LPCWSTR)strDir);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    if (CreateProcessW(nullptr, strCmd.GetBuffer(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, 10000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    strCmd.ReleaseBuffer();
}

std::wstring WordExporter::BuildContentTypes() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
           L"<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
           L"<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
           L"<Override PartName=\"/word/document.xml\""
           L" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
           L"<Override PartName=\"/word/styles.xml\""
           L" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
           L"</Types>";
}

std::wstring WordExporter::BuildRootRels() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           L"<Relationship Id=\"rId1\""
           L" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\""
           L" Target=\"word/document.xml\"/>"
           L"</Relationships>";
}

std::wstring WordExporter::BuildDocumentRels() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
           L"<Relationship Id=\"rId1\""
           L" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\""
           L" Target=\"styles.xml\"/>"
           L"</Relationships>";
}

std::wstring WordExporter::BuildDocument(const DataTable& table, const CString& strLang) const
{
    CString strTitle = table.GetSourceName();
    CString strTimestamp = GetCurrentTimestamp();

    std::wstring doc =
        L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        L"<w:document xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\""
        L" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        L"<w:body>";

    doc += L"<w:p>"
           L"<w:pPr><w:pStyle w:val=\"Title\"/></w:pPr>"
           L"<w:r><w:t>" + EscapeXml(strTitle) + L"</w:t></w:r>"
           L"</w:p>";

    doc += L"<w:p>"
           L"<w:pPr><w:pStyle w:val=\"Subtitle\"/></w:pPr>"
           L"<w:r><w:t>" + EscapeXml(strTimestamp) + L"</w:t></w:r>"
           L"</w:p>";

    wchar_t szCols[16] = {};
    _itow_s(table.GetColumnCount(), szCols, 16, 10);

    doc += L"<w:tbl>"
           L"<w:tblPr>"
           L"<w:tblStyle w:val=\"TableGrid\"/>"
           L"<w:tblW w:w=\"0\" w:type=\"auto\"/>"
           L"</w:tblPr>"
           L"<w:tblGrid>";

    for (int i = 0; i < table.GetColumnCount(); ++i)
        doc += L"<w:gridCol/>";

    doc += L"</w:tblGrid>";

    doc += BuildHeaderRow(table, strLang);
    doc += BuildDataRows(table);

    doc += L"</w:tbl>";

    doc += L"<w:sectPr/>";
    doc += L"</w:body></w:document>";

    return doc;
}

std::wstring WordExporter::BuildHeaderRow(const DataTable& table, const CString& strLang) const
{
    std::wstring row = L"<w:tr>";

    for (int i = 0; i < table.GetColumnCount(); ++i)
    {
        const DataColumn& col = table.GetColumn(i);
        CString strHeader;

        if (strLang == L"en" && !col.m_strDisplayNameEn.IsEmpty())
            strHeader = col.m_strDisplayNameEn;
        else if (!col.m_strDisplayNameKo.IsEmpty())
            strHeader = col.m_strDisplayNameKo;
        else
            strHeader = col.m_strInternalName;

        row += L"<w:tc>"
               L"<w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"4472C4\"/></w:tcPr>"
               L"<w:p><w:pPr><w:jc w:val=\"center\"/></w:pPr>"
               L"<w:r><w:rPr><w:b/><w:color w:val=\"FFFFFF\"/></w:rPr>"
               L"<w:t>" + EscapeXml(strHeader) + L"</w:t></w:r></w:p>"
               L"</w:tc>";
    }

    row += L"</w:tr>";
    return row;
}

std::wstring WordExporter::BuildDataRows(const DataTable& table) const
{
    std::wstring rows;

    for (int r = 0; r < table.GetRowCount(); ++r)
    {
        const DataRow& row = table.GetRow(r);
        std::wstring wRow = L"<w:tr>";

        for (int c = 0; c < table.GetColumnCount(); ++c)
        {
            CString strCell;
            if (c < static_cast<int>(row.m_arrCells.size()))
                strCell = row.m_arrCells[c];

            wRow += L"<w:tc>"
                    L"<w:p><w:r>"
                    L"<w:t xml:space=\"preserve\">" + EscapeXml(strCell) + L"</w:t>"
                    L"</w:r></w:p>"
                    L"</w:tc>";
        }

        wRow += L"</w:tr>";
        rows += wRow;
    }

    return rows;
}

std::wstring WordExporter::BuildStyles() const
{
    return L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           L"<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
           L"<w:style w:type=\"paragraph\" w:styleId=\"Title\">"
           L"<w:name w:val=\"Title\"/>"
           L"<w:rPr><w:b/><w:sz w:val=\"52\"/></w:rPr>"
           L"</w:style>"
           L"<w:style w:type=\"paragraph\" w:styleId=\"Subtitle\">"
           L"<w:name w:val=\"Subtitle\"/>"
           L"<w:rPr><w:color w:val=\"595959\"/><w:sz w:val=\"24\"/></w:rPr>"
           L"</w:style>"
           L"<w:style w:type=\"table\" w:styleId=\"TableGrid\">"
           L"<w:name w:val=\"Table Grid\"/>"
           L"<w:tblPr><w:tblBorders>"
           L"<w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"<w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"<w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
           L"</w:tblBorders></w:tblPr>"
           L"</w:style>"
           L"</w:styles>";
}

std::wstring WordExporter::EscapeXml(const CString& str) const
{
    std::wstring result;
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

CString WordExporter::GetCurrentTimestamp() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);

    CString strTs;
    strTs.Format(L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strTs;
}
