#include "pch.h"
#include "app/infrastructure/exporters/PdfExporter.h"
#include "app/infrastructure/exporters/HtmlReportExporter.h"
#include <vector>

BOOL PdfExporter::Export(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError)
{
    wchar_t szTempBase[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, szTempBase);

    wchar_t szTempName[MAX_PATH] = {};
    GetTempFileNameW(szTempBase, L"snx", 0, szTempName);

    CString strTempHtml = CString(szTempName) + L".html";

    HtmlReportExporter htmlExporter;
    if (!htmlExporter.Export(table, strTempHtml, strLang, strError))
    {
        DeleteFileW(szTempName);
        return FALSE;
    }

    DeleteFileW(szTempName);

    BOOL bResult = ConvertHtmlToPdf(strTempHtml, strFilePath, strError);
    DeleteFileW(strTempHtml);
    return bResult;
}

BOOL PdfExporter::ConvertHtmlToPdf(const CString& strHtmlPath, const CString& strPdfPath, CString& strError) const
{
    CString strEdgePath = FindEdgePath();
    if (strEdgePath.IsEmpty())
    {
        strError = L"Microsoft Edge를 찾을 수 없습니다. Edge가 설치되어 있는지 확인하세요.";
        return FALSE;
    }

    CString strHtmlUri = strHtmlPath;
    strHtmlUri.Replace(L"\\", L"/");

    CString strCmd;
    strCmd.Format(
        L"\"%s\" --headless --disable-gpu --no-sandbox "
        L"--print-to-pdf=\"%s\" "
        L"\"file:///%s\"",
        (LPCWSTR)strEdgePath,
        (LPCWSTR)strPdfPath,
        (LPCWSTR)strHtmlUri);

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
        strError = L"PDF 변환 프로세스를 시작할 수 없습니다.";
        return FALSE;
    }

    WaitForSingleObject(pi.hProcess, 30000);

    DWORD dwExit = 0;
    GetExitCodeProcess(pi.hProcess, &dwExit);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    DWORD dwAttr = GetFileAttributesW(strPdfPath);
    BOOL bFileExists = (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));

    if (!bFileExists)
    {
        strError = L"PDF 파일이 생성되지 않았습니다. Edge가 정상적으로 실행되었는지 확인하세요.";
        return FALSE;
    }

    return TRUE;
}

CString PdfExporter::FindEdgePath() const
{
    CString arrPaths[] = {
        L"%ProgramFiles(x86)%\\Microsoft\\Edge\\Application\\msedge.exe",
        L"%ProgramFiles%\\Microsoft\\Edge\\Application\\msedge.exe",
        L"%LocalAppData%\\Microsoft\\Edge\\Application\\msedge.exe"
    };

    for (int i = 0; i < 3; ++i)
    {
        wchar_t szExpanded[MAX_PATH] = {};
        ExpandEnvironmentStringsW(arrPaths[i], szExpanded, MAX_PATH);

        DWORD dwAttr = GetFileAttributesW(szExpanded);
        if (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            return CString(szExpanded);
    }

    return CString();
}
