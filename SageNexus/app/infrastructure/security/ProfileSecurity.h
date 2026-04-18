#pragma once
#include "pch.h"

class ProfileSecurity
{
public:
    void SetPaths(const CString& strCredentialsPath, const CString& strProfileSigPath);

    BOOL IsCredentialsFileExists() const;
    BOOL SetPassword(const CString& strPassword, CString& strError);
    BOOL VerifyPassword(const CString& strPassword) const;
    BOOL ChangePassword(const CString& strOldPassword, const CString& strNewPassword, CString& strError);

    BOOL SignProfile(const CString& strProfilePath, CString& strError) const;
    BOOL VerifyProfileSignature(const CString& strProfilePath) const;

private:
    BOOL ComputePBKDF2(
        const std::vector<BYTE>& vPassword,
        const std::vector<BYTE>& vSalt,
        DWORD                    dwIterations,
        std::vector<BYTE>&       vOutKey) const;

    BOOL ComputeHMACSHA256(
        const std::vector<BYTE>& vKey,
        const std::vector<BYTE>& vData,
        std::vector<BYTE>&       vOutMAC) const;

    CString BytesToHex(const std::vector<BYTE>& vBytes) const;
    BOOL    HexToBytes(const CString& strHex, std::vector<BYTE>& vBytes) const;
    BOOL    GenerateRandomBytes(std::vector<BYTE>& vBytes, DWORD dwCount) const;

    CString m_strCredentialsPath;
    CString m_strProfileSigPath;
};
