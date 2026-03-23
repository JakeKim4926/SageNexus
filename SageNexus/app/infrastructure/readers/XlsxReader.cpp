#include "pch.h"
#include "app/infrastructure/readers/XlsxReader.h"

BOOL XlsxReader::Read(const CString& strFilePath, DataTable& outTable, CString& strError)
{
    wchar_t szTempBase[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, szTempBase);

    wchar_t szTempFile[MAX_PATH] = {};
    GetTempFileNameW(szTempBase, L"snx", 0, szTempFile);
    DeleteFileW(szTempFile);
    CreateDirectoryW(szTempFile, nullptr);

    CString strTempDir = szTempFile;
    CString strZipCopy = strTempDir + L"\\src.zip";
    CString strExtract = strTempDir + L"\\ext";
    CreateDirectoryW(strExtract, nullptr);

    if (!CopyFileW(strFilePath, strZipCopy, FALSE))
    {
        strError = L"임시 파일 생성에 실패했습니다.";
        CleanupDir(strTempDir);
        return FALSE;
    }

    if (!ExtractZip(strZipCopy, strExtract, strError))
    {
        CleanupDir(strTempDir);
        return FALSE;
    }

    std::vector<CString> arrShared;
    CString strSS = strExtract + L"\\xl\\sharedStrings.xml";
    if (GetFileAttributesW(strSS) != INVALID_FILE_ATTRIBUTES)
        ParseSharedStrings(strSS, arrShared);

    CString strSheet = strExtract + L"\\xl\\worksheets\\sheet1.xml";
    if (GetFileAttributesW(strSheet) == INVALID_FILE_ATTRIBUTES)
    {
        strError = L"시트 데이터를 찾을 수 없습니다.";
        CleanupDir(strTempDir);
        return FALSE;
    }

    if (!ParseSheet(strSheet, arrShared, outTable, strError))
    {
        CleanupDir(strTempDir);
        return FALSE;
    }

    int nSlash1 = strFilePath.ReverseFind(L'\\');
    int nSlash2 = strFilePath.ReverseFind(L'/');
    int nSlash  = (nSlash1 > nSlash2) ? nSlash1 : nSlash2;
    outTable.SetSourceName(nSlash >= 0 ? strFilePath.Mid(nSlash + 1) : strFilePath);

    CleanupDir(strTempDir);
    return TRUE;
}

BOOL XlsxReader::ExtractZip(const CString& strZipPath, const CString& strDestDir, CString& strError) const
{
    CString strZipEsc = strZipPath;
    strZipEsc.Replace(L"'", L"''");
    CString strDestEsc = strDestDir;
    strDestEsc.Replace(L"'", L"''");

    CString strArgs;
    strArgs.Format(
        L" -NoProfile -NonInteractive -Command \"Expand-Archive -LiteralPath '%s' -DestinationPath '%s' -Force\"",
        (LPCWSTR)strZipEsc, (LPCWSTR)strDestEsc);

    CString strFull = L"powershell.exe" + strArgs;
    std::vector<wchar_t> arrCmd(strFull.GetLength() + 1, L'\0');
    wmemcpy(arrCmd.data(), strFull.GetString(), strFull.GetLength());

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(nullptr, arrCmd.data(), nullptr, nullptr,
                        FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        strError = L"PowerShell 실행에 실패했습니다.";
        return FALSE;
    }

    WaitForSingleObject(pi.hProcess, 30000);

    DWORD dwExit = 0;
    GetExitCodeProcess(pi.hProcess, &dwExit);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (dwExit != 0)
    {
        strError = L"XLSX 압축 해제에 실패했습니다.";
        return FALSE;
    }

    return TRUE;
}

BOOL XlsxReader::ParseSharedStrings(const CString& strPath, std::vector<CString>& arrStrings) const
{
    CString strXml = ReadXmlFile(strPath);
    if (strXml.IsEmpty())
        return FALSE;

    CString strOpen  = L"<si>";
    CString strClose = L"</si>";
    int nPos = 0;

    while (true)
    {
        int nStart = strXml.Find(strOpen, nPos);
        if (nStart < 0) break;
        int nEnd = strXml.Find(strClose, nStart);
        if (nEnd < 0) break;

        CString strSi = strXml.Mid(nStart, nEnd + strClose.GetLength() - nStart);
        arrStrings.push_back(UnescapeXml(GetAllTagContent(strSi, L"t")));
        nPos = nEnd + strClose.GetLength();
    }
    return TRUE;
}

BOOL XlsxReader::ParseSheet(const CString& strPath, const std::vector<CString>& arrShared,
                             DataTable& outTable, CString& strError) const
{
    CString strXml = ReadXmlFile(strPath);
    if (strXml.IsEmpty())
    {
        strError = L"시트 XML을 읽을 수 없습니다.";
        return FALSE;
    }

    outTable.Clear();

    std::vector<CString> arrRows = FindRows(strXml);
    if (arrRows.empty())
        return TRUE;

    std::vector<CString> arrHeaderCells = FindCells(arrRows[0]);

    int nColCount = 0;
    for (const CString& strCell : arrHeaderCells)
    {
        int nIdx = ColRefToIndex(GetAttribute(strCell, L"r"));
        if (nIdx + 1 > nColCount)
            nColCount = nIdx + 1;
    }
    if (nColCount == 0)
        nColCount = (int)arrHeaderCells.size();

    std::vector<CString> arrHeaderValues(nColCount);
    for (const CString& strCell : arrHeaderCells)
    {
        int nIdx = ColRefToIndex(GetAttribute(strCell, L"r"));
        if (nIdx >= 0 && nIdx < nColCount)
            arrHeaderValues[nIdx] = UnescapeXml(GetCellValue(strCell, arrShared));
    }

    for (int i = 0; i < nColCount; ++i)
    {
        CString strName = arrHeaderValues[i];
        if (strName.IsEmpty())
            strName.Format(L"col_%d", i);

        DataColumn col;
        col.m_strInternalName  = strName;
        col.m_strSourceName    = strName;
        col.m_strDisplayNameKo = strName;
        col.m_strDataType      = L"string";
        outTable.AddColumn(col);
    }

    for (int r = 1; r < (int)arrRows.size(); ++r)
    {
        std::vector<CString> arrCells = FindCells(arrRows[r]);

        DataRow row;
        row.m_arrCells.resize(nColCount);

        for (const CString& strCell : arrCells)
        {
            int nIdx = ColRefToIndex(GetAttribute(strCell, L"r"));
            if (nIdx >= 0 && nIdx < nColCount)
                row.m_arrCells[nIdx] = UnescapeXml(GetCellValue(strCell, arrShared));
        }

        outTable.AddRow(row);
    }

    return TRUE;
}

void XlsxReader::CleanupDir(const CString& strDir) const
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

CString XlsxReader::ReadXmlFile(const CString& strPath) const
{
    HANDLE hFile = CreateFileW(strPath, GENERIC_READ, FILE_SHARE_READ,
                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return L"";

    DWORD dwSize = GetFileSize(hFile, nullptr);
    if (dwSize == 0 || dwSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return L"";
    }

    std::vector<char> arrBytes(dwSize + 1, 0);
    DWORD dwRead = 0;
    ReadFile(hFile, arrBytes.data(), dwSize, &dwRead, nullptr);
    CloseHandle(hFile);

    char* pData   = arrBytes.data();
    DWORD nLen    = dwRead;
    if (nLen >= 3 &&
        (unsigned char)pData[0] == 0xEF &&
        (unsigned char)pData[1] == 0xBB &&
        (unsigned char)pData[2] == 0xBF)
    {
        pData += 3;
        nLen  -= 3;
    }

    int nWide = MultiByteToWideChar(CP_UTF8, 0, pData, (int)nLen, nullptr, 0);
    if (nWide <= 0)
        return L"";

    std::vector<wchar_t> arrWide(nWide + 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, pData, (int)nLen, arrWide.data(), nWide);
    return CString(arrWide.data());
}

std::vector<CString> XlsxReader::FindRows(const CString& strXml) const
{
    std::vector<CString> arrRows;
    CString strOpen  = L"<row ";
    CString strClose = L"</row>";
    int nPos = 0;

    while (true)
    {
        int nStart = strXml.Find(strOpen, nPos);
        if (nStart < 0) break;
        int nEnd = strXml.Find(strClose, nStart);
        if (nEnd < 0) break;

        int nEndFull = nEnd + strClose.GetLength();
        arrRows.push_back(strXml.Mid(nStart, nEndFull - nStart));
        nPos = nEndFull;
    }
    return arrRows;
}

std::vector<CString> XlsxReader::FindCells(const CString& strRowXml) const
{
    std::vector<CString> arrCells;
    CString strOpen  = L"<c ";
    CString strClose = L"</c>";
    int nPos = 0;

    while (true)
    {
        int nStart = strRowXml.Find(strOpen, nPos);
        if (nStart < 0) break;

        int nCloseTag  = strRowXml.Find(strClose, nStart);
        int nNextCell  = strRowXml.Find(strOpen, nStart + 1);
        int nSearchEnd = (nNextCell >= 0) ? nNextCell : strRowXml.GetLength();

        int nSelfClose = -1;
        for (int i = nStart; i < nSearchEnd - 1; ++i)
        {
            if (strRowXml[i] == L'/' && strRowXml[i + 1] == L'>')
            {
                nSelfClose = i;
                break;
            }
        }

        int nEnd;
        if (nSelfClose >= 0 && (nCloseTag < 0 || nSelfClose < nCloseTag))
            nEnd = nSelfClose + 2;
        else if (nCloseTag >= 0)
            nEnd = nCloseTag + strClose.GetLength();
        else
            break;

        arrCells.push_back(strRowXml.Mid(nStart, nEnd - nStart));
        nPos = nEnd;
    }
    return arrCells;
}

CString XlsxReader::GetAttribute(const CString& strElement, const CString& strAttr) const
{
    CString strSearch = strAttr + L"=\"";
    int nPos = strElement.Find(strSearch);
    if (nPos < 0) return L"";

    int nStart = nPos + strSearch.GetLength();
    int nEnd   = strElement.Find(L'"', nStart);
    if (nEnd < 0) return L"";

    return strElement.Mid(nStart, nEnd - nStart);
}

CString XlsxReader::GetTagContent(const CString& strXml, const CString& strTag) const
{
    CString strOpen  = L"<" + strTag;
    CString strClose = L"</" + strTag + L">";

    int nStart = strXml.Find(strOpen);
    if (nStart < 0) return L"";

    int nTagEnd = strXml.Find(L'>', nStart);
    if (nTagEnd < 0) return L"";

    if (nTagEnd > 0 && strXml[nTagEnd - 1] == L'/')
        return L"";

    int nContentStart = nTagEnd + 1;
    int nEnd = strXml.Find(strClose, nContentStart);
    if (nEnd < 0) return L"";

    return strXml.Mid(nContentStart, nEnd - nContentStart);
}

CString XlsxReader::GetAllTagContent(const CString& strXml, const CString& strTag) const
{
    CString strOpen  = L"<" + strTag;
    CString strClose = L"</" + strTag + L">";
    CString strResult;
    int nPos = 0;

    while (true)
    {
        int nStart = strXml.Find(strOpen, nPos);
        if (nStart < 0) break;

        int nTagEnd = strXml.Find(L'>', nStart);
        if (nTagEnd < 0) break;

        if (nTagEnd > 0 && strXml[nTagEnd - 1] == L'/')
        {
            nPos = nTagEnd + 1;
            continue;
        }

        int nContentStart = nTagEnd + 1;
        int nEnd = strXml.Find(strClose, nContentStart);
        if (nEnd < 0) break;

        strResult += strXml.Mid(nContentStart, nEnd - nContentStart);
        nPos = nEnd + strClose.GetLength();
    }
    return strResult;
}

CString XlsxReader::UnescapeXml(const CString& str) const
{
    CString s = str;
    s.Replace(L"&amp;",  L"&");
    s.Replace(L"&lt;",   L"<");
    s.Replace(L"&gt;",   L">");
    s.Replace(L"&quot;", L"\"");
    s.Replace(L"&apos;", L"'");
    return s;
}

int XlsxReader::ColRefToIndex(const CString& strCellRef) const
{
    if (strCellRef.IsEmpty()) return -1;

    int nLetters = 0;
    while (nLetters < strCellRef.GetLength() && iswupper(strCellRef[nLetters]))
        ++nLetters;

    if (nLetters == 0) return -1;

    int nResult = 0;
    for (int i = 0; i < nLetters; ++i)
        nResult = nResult * 26 + (strCellRef[i] - L'A' + 1);

    return nResult - 1;
}

CString XlsxReader::GetCellValue(const CString& strCellXml, const std::vector<CString>& arrShared) const
{
    CString strType = GetAttribute(strCellXml, L"t");

    if (strType == L"s")
    {
        CString strIdx = GetTagContent(strCellXml, L"v");
        if (strIdx.IsEmpty()) return L"";
        int nIdx = _wtoi(strIdx);
        if (nIdx >= 0 && nIdx < (int)arrShared.size())
            return arrShared[nIdx];
        return strIdx;
    }

    if (strType == L"inlineStr")
        return GetAllTagContent(strCellXml, L"t");

    if (strType == L"b")
    {
        CString strV = GetTagContent(strCellXml, L"v");
        return (strV == L"1") ? L"TRUE" : L"FALSE";
    }

    if (strType == L"e")
        return GetTagContent(strCellXml, L"v");

    return GetTagContent(strCellXml, L"v");
}
