#include "pch.h"
#include "app/host/LoginDialog.h"
#include "resources/resource.h"

BOOL LoginDialog::ShowLogin(HINSTANCE hInstance, ProfileSecurity& security)
{
    LoginDialogData data;
    data.pSecurity       = &security;
    data.bSetPasswordMode = FALSE;

    INT_PTR nResult = DialogBoxParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_LOGIN),
        nullptr,
        DialogProc,
        reinterpret_cast<LPARAM>(&data));

    return (nResult == IDOK) ? TRUE : FALSE;
}

BOOL LoginDialog::ShowSetPassword(HINSTANCE hInstance, ProfileSecurity& security)
{
    LoginDialogData data;
    data.pSecurity       = &security;
    data.bSetPasswordMode = TRUE;

    INT_PTR nResult = DialogBoxParamW(
        hInstance,
        MAKEINTRESOURCEW(IDD_SET_PASSWORD),
        nullptr,
        DialogProc,
        reinterpret_cast<LPARAM>(&data));

    return (nResult == IDOK) ? TRUE : FALSE;
}

INT_PTR CALLBACK LoginDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtrW(hDlg, DWLP_USER, lParam);
        return TRUE;

    case WM_COMMAND:
        {
            LoginDialogData* pData = reinterpret_cast<LoginDialogData*>(
                GetWindowLongPtrW(hDlg, DWLP_USER));
            if (!pData)
                return FALSE;

            WORD wId = LOWORD(wParam);

            if (wId == IDOK)
            {
                if (pData->bSetPasswordMode)
                    OnOkSetPassword(hDlg, pData);
                else
                    OnOkLogin(hDlg, pData);
                return TRUE;
            }

            if (wId == IDCANCEL)
            {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

void LoginDialog::OnOkLogin(HWND hDlg, LoginDialogData* pData)
{
    wchar_t szPassword[256] = {};
    GetDlgItemTextW(hDlg, IDC_EDIT_PASSWORD, szPassword, 256);

    CString strPassword(szPassword);
    SecureZeroMemory(szPassword, sizeof(szPassword));

    if (!pData->pSecurity->VerifyPassword(strPassword))
    {
        ShowError(hDlg, L"비밀번호가 올바르지 않습니다.");
        SetDlgItemTextW(hDlg, IDC_EDIT_PASSWORD, L"");
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_PASSWORD));
        return;
    }

    EndDialog(hDlg, IDOK);
}

void LoginDialog::OnOkSetPassword(HWND hDlg, LoginDialogData* pData)
{
    wchar_t szPassword[256] = {};
    wchar_t szConfirm[256]  = {};

    GetDlgItemTextW(hDlg, IDC_EDIT_PASSWORD, szPassword, 256);
    GetDlgItemTextW(hDlg, IDC_EDIT_CONFIRM,  szConfirm,  256);

    CString strPassword(szPassword);
    CString strConfirm(szConfirm);
    SecureZeroMemory(szPassword, sizeof(szPassword));
    SecureZeroMemory(szConfirm,  sizeof(szConfirm));

    if (strPassword.IsEmpty())
    {
        ShowError(hDlg, L"비밀번호를 입력해주세요.");
        return;
    }

    if (strPassword != strConfirm)
    {
        ShowError(hDlg, L"비밀번호가 일치하지 않습니다.");
        SetDlgItemTextW(hDlg, IDC_EDIT_CONFIRM, L"");
        SetFocus(GetDlgItem(hDlg, IDC_EDIT_CONFIRM));
        return;
    }

    CString strError;
    if (!pData->pSecurity->SetPassword(strPassword, strError))
    {
        ShowError(hDlg, strError);
        return;
    }

    EndDialog(hDlg, IDOK);
}

void LoginDialog::ShowError(HWND hDlg, const CString& strMessage)
{
    SetDlgItemTextW(hDlg, IDC_STATIC_ERROR, strMessage);
}
