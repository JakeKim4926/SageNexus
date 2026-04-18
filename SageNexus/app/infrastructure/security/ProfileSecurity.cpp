#include "pch.h"
#include "app/infrastructure/security/ProfileSecurity.h"
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

static const BYTE HMAC_APP_KEY[32] = {
    0x53, 0x61, 0x67, 0x65, 0x4E, 0x65, 0x78, 0x75,
    0x73, 0x2D, 0x50, 0x72, 0x6F, 0x66, 0x69, 0x6C,
    0x65, 0x2D, 0x49, 0x6E, 0x74, 0x65, 0x67, 0x72,
    0x69, 0x74, 0x79, 0x2D, 0x4B, 0x65, 0x79, 0x21
};

constexpr DWORD PBKDF2_ITER_COUNT = 100000;
constexpr DWORD SALT_BYTE_LENGTH  = 32;
constexpr DWORD HASH_BYTE_LENGTH  = 32;

void ProfileSecurity::SetPaths(const CString& strCredentialsPath, const CString& strProfileSigPath)
{
    m_strCredentialsPath = strCredentialsPath;
    m_strProfileSigPath  = strProfileSigPath;
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

BOOL ProfileSecurity::SignProfile(const CString& strProfilePath, CString& strError) const
{
    std::string strPath = WideToUtf8(strProfilePath);
    std::ifstream file(strPath, std::ios::binary);
    if (!file.is_open())
    {
        strError = L"프로필 파일을 읽을 수 없습니다.";
        return FALSE;
    }

    std::vector<BYTE> vData(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    std::vector<BYTE> vKey(HMAC_APP_KEY, HMAC_APP_KEY + 32);
    std::vector<BYTE> vMAC;

    if (!ComputeHMACSHA256(vKey, vData, vMAC))
    {
        strError = L"프로필 서명에 실패했습니다.";
        return FALSE;
    }

    std::string strSigPath = WideToUtf8(m_strProfileSigPath);
    std::ofstream sigFile(strSigPath, std::ios::out | std::ios::trunc);
    if (!sigFile.is_open())
    {
        strError = L"서명 파일 저장에 실패했습니다.";
        return FALSE;
    }

    sigFile << "HMAC-SHA256\n";
    sigFile << WideToUtf8(BytesToHex(vMAC)) << "\n";

    return TRUE;
}

BOOL ProfileSecurity::VerifyProfileSignature(const CString& strProfilePath) const
{
    std::string strSigPath = WideToUtf8(m_strProfileSigPath);
    std::ifstream sigFile(strSigPath);
    if (!sigFile.is_open())
        return FALSE;

    std::string strAlgorithm, strStoredHex;
    if (!std::getline(sigFile, strAlgorithm)) return FALSE;
    if (!std::getline(sigFile, strStoredHex)) return FALSE;

    if (strAlgorithm != "HMAC-SHA256")
        return FALSE;

    std::vector<BYTE> vStoredMAC;
    if (!HexToBytes(Utf8ToWide(strStoredHex), vStoredMAC)) return FALSE;

    std::string strPath = WideToUtf8(strProfilePath);
    std::ifstream profileFile(strPath, std::ios::binary);
    if (!profileFile.is_open()) return FALSE;

    std::vector<BYTE> vData(
        (std::istreambuf_iterator<char>(profileFile)),
        std::istreambuf_iterator<char>());

    std::vector<BYTE> vKey(HMAC_APP_KEY, HMAC_APP_KEY + 32);
    std::vector<BYTE> vComputedMAC;

    if (!ComputeHMACSHA256(vKey, vData, vComputedMAC))
        return FALSE;

    if (vComputedMAC.size() != vStoredMAC.size())
        return FALSE;

    BYTE nResult = 0;
    for (size_t i = 0; i < vComputedMAC.size(); ++i)
        nResult |= vComputedMAC[i] ^ vStoredMAC[i];

    return (nResult == 0) ? TRUE : FALSE;
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

BOOL ProfileSecurity::ComputeHMACSHA256(
    const std::vector<BYTE>& vKey,
    const std::vector<BYTE>& vData,
    std::vector<BYTE>& vOutMAC) const
{
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA256_ALGORITHM,
        nullptr,
        BCRYPT_ALG_HANDLE_HMAC_FLAG);

    if (!BCRYPT_SUCCESS(status))
        return FALSE;

    BCRYPT_HASH_HANDLE hHash = nullptr;
    status = BCryptCreateHash(
        hAlg,
        &hHash,
        nullptr, 0,
        const_cast<PUCHAR>(vKey.data()),
        static_cast<ULONG>(vKey.size()),
        0);

    if (!BCRYPT_SUCCESS(status))
    {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return FALSE;
    }

    if (!vData.empty())
    {
        status = BCryptHashData(
            hHash,
            const_cast<PUCHAR>(vData.data()),
            static_cast<ULONG>(vData.size()),
            0);

        if (!BCRYPT_SUCCESS(status))
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return FALSE;
        }
    }

    vOutMAC.resize(32);
    status = BCryptFinishHash(
        hHash,
        vOutMAC.data(),
        static_cast<ULONG>(vOutMAC.size()),
        0);

    BCryptDestroyHash(hHash);
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
