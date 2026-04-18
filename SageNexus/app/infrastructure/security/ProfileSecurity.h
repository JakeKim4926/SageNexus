#pragma once
#include "pch.h"

class ProfileSecurity
{
public:
    void SetCredentialsPath(const CString& strCredentialsPath);

    BOOL IsCredentialsFileExists() const;
    BOOL SetPassword(const CString& strPassword, CString& strError);
    BOOL VerifyPassword(const CString& strPassword) const;
    BOOL ChangePassword(const CString& strOldPassword, const CString& strNewPassword, CString& strError);

private:
    BOOL ComputePBKDF2(
        const std::vector<BYTE>& vPassword,
        const std::vector<BYTE>& vSalt,
        DWORD                    dwIterations,
        std::vector<BYTE>&       vOutKey) const;

    CString BytesToHex(const std::vector<BYTE>& vBytes) const;
    BOOL    HexToBytes(const CString& strHex, std::vector<BYTE>& vBytes) const;
    BOOL    GenerateRandomBytes(std::vector<BYTE>& vBytes, DWORD dwCount) const;

    CString m_strCredentialsPath;
};
