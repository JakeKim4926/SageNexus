#include "pch.h"
#include "app/application/services/EmailService.h"
#include <mapi.h>

BOOL EmailService::SendEmail(const EmailAction& action, CString& strError)
{
    if (action.m_strRecipients.IsEmpty())
    {
        strError = L"수신자 주소가 비어 있습니다.";
        return FALSE;
    }

    HMODULE hMapi = LoadLibraryW(L"mapi32.dll");
    if (!hMapi)
    {
        strError = L"MAPI 라이브러리를 로드할 수 없습니다. 이메일 클라이언트가 설치되어 있는지 확인하세요.";
        return FALSE;
    }

    typedef ULONG (PASCAL *LPMAPISENDMAILW)(LHANDLE, ULONG_PTR, lpMapiMessageW, FLAGS, ULONG);
    LPMAPISENDMAILW pfnSendMailW = (LPMAPISENDMAILW)GetProcAddress(hMapi, "MAPISendMailW");

    if (!pfnSendMailW)
    {
        FreeLibrary(hMapi);
        strError = L"MAPISendMailW 함수를 찾을 수 없습니다. 최신 이메일 클라이언트가 필요합니다.";
        return FALSE;
    }

    std::vector<CString>        arrRecipAddrs = ParseRecipients(action.m_strRecipients);
    std::vector<MapiRecipDescW> arrRecips;
    arrRecips.reserve(arrRecipAddrs.size());

    for (int i = 0; i < (int)arrRecipAddrs.size(); ++i)
    {
        MapiRecipDescW recip = {};
        recip.ulRecipClass   = MAPI_TO;
        recip.lpszName       = (LPWSTR)(LPCWSTR)arrRecipAddrs[i];
        recip.lpszAddress    = (LPWSTR)(LPCWSTR)arrRecipAddrs[i];
        arrRecips.push_back(recip);
    }

    if (arrRecips.empty())
    {
        FreeLibrary(hMapi);
        strError = L"유효한 수신자 주소가 없습니다.";
        return FALSE;
    }

    CString strAttachName;
    MapiFileDescW fileDesc = {};
    if (!action.m_strAttachFilePath.IsEmpty())
    {
        int nSlash = action.m_strAttachFilePath.ReverseFind(L'\\');
        if (nSlash < 0)
            nSlash = action.m_strAttachFilePath.ReverseFind(L'/');
        strAttachName = (nSlash >= 0) ? action.m_strAttachFilePath.Mid(nSlash + 1) : action.m_strAttachFilePath;

        fileDesc.lpszPathName = (LPWSTR)(LPCWSTR)action.m_strAttachFilePath;
        fileDesc.lpszFileName = (LPWSTR)(LPCWSTR)strAttachName;
    }

    MapiMessageW mapiMsg   = {};
    mapiMsg.lpszSubject    = (LPWSTR)(LPCWSTR)action.m_strSubject;
    mapiMsg.lpszNoteText   = (LPWSTR)(LPCWSTR)action.m_strBody;
    mapiMsg.nRecipCount    = (ULONG)arrRecips.size();
    mapiMsg.lpRecips       = arrRecips.data();

    if (!action.m_strAttachFilePath.IsEmpty())
    {
        mapiMsg.nFileCount = 1;
        mapiMsg.lpFiles    = &fileDesc;
    }

    ULONG nResult = pfnSendMailW(0, 0, &mapiMsg, MAPI_LOGON_UI, 0);

    FreeLibrary(hMapi);

    if (nResult != SUCCESS_SUCCESS)
    {
        wchar_t szCode[32] = {};
        _itow_s((int)nResult, szCode, 32, 10);
        strError = L"이메일 발송 실패 (MAPI 오류 코드: ";
        strError += szCode;
        strError += L")";
        return FALSE;
    }

    return TRUE;
}

std::vector<CString> EmailService::ParseRecipients(const CString& strRecipients) const
{
    std::vector<CString> arr;
    CString s = strRecipients;
    s.Replace(L',', L';');

    int nPos = 0;
    while (nPos <= s.GetLength())
    {
        int nSemi = s.Find(L';', nPos);
        CString strPart;
        if (nSemi < 0)
            strPart = s.Mid(nPos);
        else
            strPart = s.Mid(nPos, nSemi - nPos);

        strPart.Trim();
        if (!strPart.IsEmpty())
            arr.push_back(strPart);

        if (nSemi < 0) break;
        nPos = nSemi + 1;
    }
    return arr;
}
