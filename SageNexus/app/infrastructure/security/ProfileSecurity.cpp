#include "pch.h"
#include "app/infrastructure/security/ProfileSecurity.h"
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

constexpr DWORD PBKDF2_ITER_COUNT = 100000;
constexpr DWORD SALT_BYTE_LENGTH  = 32;
constexpr DWORD HASH_BYTE_LENGTH  = 32;

void ProfileSecurity::SetCredentialsPath(const CString& strCredentialsPath)
{
    m_strCredentialsPath = strCredentialsPath;
}

BOOL ProfileSecurity::IsCredentialsFileExists() const
{
    return (GetFileAttributesW(m_strCredentialsPath) != INVALID_FILE_ATTRIBUTES) ? TRUE : FALSE;
}

BOOL ProfileSecurity::SetPassword(const CString& strPassword, CString& strError)
{
    if (strPassword.IsEmpty())
    {
        strError = L"비밀번호를 입력해주세요.";
        return FALSE;
    }

    std::vector<BYTE> vSalt;
    if (!GenerateRandomBytes(vSalt, SALT_BYTE_LENGTH))
    {
        strError = L"보안 데이터 생성에 실패했습니다.";
        return FALSE;
    }

    std::string strPwUtf8 = WideToUtf8(strPassword);
    std::vector<BYTE> vPassword(strPwUtf8.begin(), strPwUtf8.end());
    std::vector<BYTE> vHash;

    if (!ComputePBKDF2(vPassword, vSalt, PBKDF2_ITER_COUNT, vHash))
    {
        strError = L"비밀번호 처리에 실패했습니다.";
        return FALSE;
    }

    std::string strPath = WideToUtf8(m_strCredentialsPath);
    std::ofstream file(strPath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        strError = L"자격증명 파일 저장에 실패했습니다.";
        return FALSE;
    }

    file << "PBKDF2-SHA256\n";
    file << PBKDF2_ITER_COUNT << "\n";
    file << WideToUtf8(BytesToHex(vSalt)) << "\n";
    file << WideToUtf8(BytesToHex(vHash)) << "\n";

    return TRUE;
}

BOOL ProfileSecurity::VerifyPassword(const CString& strPassword) const
{
    std::string strPath = WideToUtf8(m_strCredentialsPath);
    std::ifstream file(strPath);
    if (!file.is_open())
        return FALSE;

    std::string strAlgorithm, strIterStr, strSaltHex, strHashHex;
    if (!std::getline(file, strAlgorithm)) return FALSE;
    if (!std::getline(file, strIterStr))   return FALSE;
    if (!std::getline(file, strSaltHex))   return FALSE;
    if (!std::getline(file, strHashHex))   return FALSE;

    if (strAlgorithm != "PBKDF2-SHA256")
        return FALSE;

    DWORD dwIterations = static_cast<DWORD>(std::stoul(strIterStr));

    std::vector<BYTE> vSalt;
    if (!HexToBytes(Utf8ToWide(strSaltHex), vSalt)) return FALSE;

    std::vector<BYTE> vStoredHash;
    if (!HexToBytes(Utf8ToWide(strHashHex), vStoredHash)) return FALSE;

    std::string strPwUtf8 = WideToUtf8(strPassword);
    std::vector<BYTE> vPassword(strPwUtf8.begin(), strPwUtf8.end());
    std::vector<BYTE> vComputedHash;

    if (!ComputePBKDF2(vPassword, vSalt, dwIterations, vComputedHash))
        return FALSE;

    if (vComputedHash.size() != vStoredHash.size())
        return FALSE;

    BYTE nResult = 0;
    for (size_t i = 0; i < vComputedHash.size(); ++i)
        nResult |= vComputedHash[i] ^ vStoredHash[i];

    return (nResult == 0) ? TRUE : FALSE;
}

BOOL ProfileSecurity::ChangePassword(
    const CString& strOldPassword,
    const CString& strNewPassword,
    CString& strError)
{
    if (!VerifyPassword(strOldPassword))
    {
        strError = L"현재 비밀번호가 올바르지 않습니다.";
        return FALSE;
    }
    return SetPassword(strNewPassword, strError);
}

BOOL ProfileSecurity::ComputePBKDF2(
    const std::vector<BYTE>& vPassword,
    const std::vector<BYTE>& vSalt,
    DWORD dwIterations,
    std::vector<BYTE>& vOutKey) const
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA256_ALGORITHM,
        nullptr,
        BCRYPT_ALG_HANDLE_HMAC_FLAG);

    if (!BCRYPT_SUCCESS(status))
        return FALSE;

    vOutKey.resize(HASH_BYTE_LENGTH);
    status = BCryptDeriveKeyPBKDF2(
        hAlg,
        const_cast<PUCHAR>(vPassword.empty() ? nullptr : vPassword.data()),
        static_cast<ULONG>(vPassword.size()),
        const_cast<PUCHAR>(vSalt.data()),
        static_cast<ULONG>(vSalt.size()),
        dwIterations,
        vOutKey.data(),
        static_cast<ULONG>(vOutKey.size()),
        0);

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return BCRYPT_SUCCESS(status) ? TRUE : FALSE;
}

CString ProfileSecurity::BytesToHex(const std::vector<BYTE>& vBytes) const
{
    CString strResult;
    for (BYTE b : vBytes)
    {
        CString strByte;
        strByte.Format(L"%02x", b);
        strResult += strByte;
    }
    return strResult;
}

BOOL ProfileSecurity::HexToBytes(const CString& strHex, std::vector<BYTE>& vBytes) const
{
    if (strHex.GetLength() % 2 != 0)
        return FALSE;

    vBytes.clear();
    for (int i = 0; i < strHex.GetLength(); i += 2)
    {
        CString strByte = strHex.Mid(i, 2);
        unsigned int nByte = 0;
        if (swscanf_s((LPCWSTR)strByte, L"%02x", &nByte) != 1)
            return FALSE;
        vBytes.push_back(static_cast<BYTE>(nByte));
    }
    return TRUE;
}

BOOL ProfileSecurity::GenerateRandomBytes(std::vector<BYTE>& vBytes, DWORD dwCount) const
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_RNG_ALGORITHM,
        nullptr,
        0);

    if (!BCRYPT_SUCCESS(status))
        return FALSE;

    vBytes.resize(dwCount);
    status = BCryptGenRandom(hAlg, vBytes.data(), dwCount, 0);

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return BCRYPT_SUCCESS(status) ? TRUE : FALSE;
}
