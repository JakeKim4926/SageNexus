#pragma once
#include "pch.h"
#include "app/infrastructure/security/ProfileSecurity.h"

class LoginDialog
{
public:
    static BOOL ShowLogin(HINSTANCE hInstance, ProfileSecurity& security);
    static BOOL ShowSetPassword(HINSTANCE hInstance, ProfileSecurity& security);

private:
    struct LoginDialogData
    {
        ProfileSecurity* pSecurity;
        BOOL             bSetPasswordMode;
    };

    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void OnOkLogin(HWND hDlg, LoginDialogData* pData);
    static void OnOkSetPassword(HWND hDlg, LoginDialogData* pData);
    static void ShowError(HWND hDlg, const CString& strMessage);
};
