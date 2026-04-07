#include "pch.h"
#include "app/infrastructure/history/ArtifactStore.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <string>

BOOL ArtifactStore::SaveArtifact(Artifact& artifact, CString& strError)
{
    artifact.m_strId        = GenerateId();
    artifact.m_strCreatedAt = GetCurrentTimestamp();

    if (!EnsureDataDirectory(strError))
        return FALSE;

    CString strFilePath = BuildFilePath();

    std::vector<Artifact> arrArtifacts;
    CString strExisting = ReadFileAsWideString(strFilePath);
    if (!strExisting.IsEmpty())
    {
        CString strParseError;
        ParseArtifacts(strExisting, arrArtifacts, strParseError);
    }

    arrArtifacts.push_back(artifact);

    CString strJson = SerializeArtifacts(arrArtifacts);
    return WriteFileAsUtf8(strFilePath, strJson, strError);
}

BOOL ArtifactStore::LoadArtifacts(std::vector<Artifact>& arrArtifacts, CString& strError) const
{
    CString strFilePath = BuildFilePath();
    CString strJson     = ReadFileAsWideString(strFilePath);
    if (strJson.IsEmpty())
        return TRUE;

    return ParseArtifacts(strJson, arrArtifacts, strError);
}

CString ArtifactStore::BuildFilePath() const
{
    return sageMgr.GetDataDir() + L"\\artifacts.json";
}

BOOL ArtifactStore::EnsureDataDirectory(CString& strError) const
{
    CString strDataDir = sageMgr.GetDataDir();
    if (!CreateDirectoryW(strDataDir, nullptr))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS)
        {
            strError = L"데이터 디렉토리 생성 실패";
            return FALSE;
        }
    }
    return TRUE;
}

CString ArtifactStore::GenerateId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"art-%04d%02d%02d%02d%02d%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strId;
}

CString ArtifactStore::GetCurrentTimestamp() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strTs;
    strTs.Format(L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strTs;
}

CString ArtifactStore::SerializeArtifacts(const std::vector<Artifact>& arrArtifacts) const
{
    CString strJson = L"[";
    for (int i = 0; i < (int)arrArtifacts.size(); ++i)
    {
        if (i > 0) strJson += L",";
        strJson += SerializeArtifact(arrArtifacts[i]);
    }
    strJson += L"]";
    return strJson;
}

CString ArtifactStore::SerializeArtifact(const Artifact& artifact) const
{
    wchar_t buf[32] = {};
    CString strJson = L"{";
    strJson += L"\"id\":\""         + EscapeJsonString(artifact.m_strId)         + L"\",";
    strJson += L"\"sourceName\":\"" + EscapeJsonString(artifact.m_strSourceName) + L"\",";
    strJson += L"\"filePath\":\""   + EscapeJsonString(artifact.m_strFilePath)   + L"\",";
    strJson += L"\"format\":\""     + EscapeJsonString(artifact.m_strFormat)     + L"\",";
    strJson += L"\"createdAt\":\""  + EscapeJsonString(artifact.m_strCreatedAt)  + L"\",";
    swprintf_s(buf, 32, L"%d", artifact.m_nRowCount);
    strJson += L"\"rowCount\":"; strJson += buf; strJson += L",";
    swprintf_s(buf, 32, L"%d", artifact.m_nColumnCount);
    strJson += L"\"columnCount\":"; strJson += buf;
    strJson += L"}";
    return strJson;
}

BOOL ArtifactStore::ParseArtifacts(const CString& strJson, std::vector<Artifact>& arrArtifacts, CString& strError) const
{
    int nStart = strJson.Find(L'[');
    int nEnd   = strJson.ReverseFind(L']');
    if (nStart < 0 || nEnd < 0 || nEnd <= nStart)
        return TRUE;

    CString strContent = strJson.Mid(nStart + 1, nEnd - nStart - 1);
    std::vector<CString> arrObjects = SplitJsonObjects(strContent);

    for (const CString& strObj : arrObjects)
    {
        if (strObj.IsEmpty())
            continue;

        Artifact art;
        art.m_strId          = UnescapeJsonString(ExtractStringField(strObj, L"id"));
        art.m_strSourceName  = UnescapeJsonString(ExtractStringField(strObj, L"sourceName"));
        art.m_strFilePath    = UnescapeJsonString(ExtractStringField(strObj, L"filePath"));
        art.m_strFormat      = UnescapeJsonString(ExtractStringField(strObj, L"format"));
        art.m_strCreatedAt   = UnescapeJsonString(ExtractStringField(strObj, L"createdAt"));
        art.m_nRowCount      = ExtractIntField(strObj, L"rowCount");
        art.m_nColumnCount   = ExtractIntField(strObj, L"columnCount");
        arrArtifacts.push_back(art);
    }
    return TRUE;
}

CString ArtifactStore::EscapeJsonString(const CString& str) const
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        wchar_t ch = str[i];
        switch (ch)
        {
        case L'"':  strResult += L"\\\""; break;
        case L'\\': strResult += L"\\\\"; break;
        case L'\n': strResult += L"\\n";  break;
        case L'\r': strResult += L"\\r";  break;
        case L'\t': strResult += L"\\t";  break;
        default:    strResult += ch;      break;
        }
    }
    return strResult;
}

CString ArtifactStore::UnescapeJsonString(const CString& str) const
{
    CString strResult;
    for (int i = 0; i < str.GetLength(); ++i)
    {
        if (str[i] == L'\\' && i + 1 < str.GetLength())
        {
            ++i;
            switch (str[i])
            {
            case L'"':  strResult += L'"';  break;
            case L'\\': strResult += L'\\'; break;
            case L'/':  strResult += L'/';  break;
            case L'n':  strResult += L'\n'; break;
            case L'r':  strResult += L'\r'; break;
            case L't':  strResult += L'\t'; break;
            default:    strResult += str[i]; break;
            }
        }
        else
        {
            strResult += str[i];
        }
    }
    return strResult;
}

CString ArtifactStore::ExtractStringField(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0) return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0) return L"";

    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    if (nValueStart >= strJson.GetLength() || strJson[nValueStart] != L'"')
        return L"";

    int nEnd = nValueStart + 1;
    while (nEnd < strJson.GetLength())
    {
        if (strJson[nEnd] == L'\\') { nEnd += 2; continue; }
        if (strJson[nEnd] == L'"')  break;
        ++nEnd;
    }

    if (nEnd >= strJson.GetLength())
        return L"";

    return strJson.Mid(nValueStart + 1, nEnd - nValueStart - 1);
}

int ArtifactStore::ExtractIntField(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0) return 0;

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0) return 0;

    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    int nEnd = nValueStart;
    while (nEnd < strJson.GetLength() && strJson[nEnd] >= L'0' && strJson[nEnd] <= L'9')
        ++nEnd;

    if (nEnd == nValueStart) return 0;
    return _wtoi(strJson.Mid(nValueStart, nEnd - nValueStart));
}

std::vector<CString> ArtifactStore::SplitJsonObjects(const CString& strContent) const
{
    std::vector<CString> arrResult;
    int nDepth = 0;
    int nStart = -1;

    for (int i = 0; i < strContent.GetLength(); ++i)
    {
        wchar_t ch = strContent[i];

        if (ch == L'"')
        {
            ++i;
            while (i < strContent.GetLength())
            {
                if (strContent[i] == L'\\') ++i;
                else if (strContent[i] == L'"') break;
                ++i;
            }
            continue;
        }

        if (ch == L'{')
        {
            if (nDepth == 0) nStart = i;
            ++nDepth;
        }
        else if (ch == L'}')
        {
            --nDepth;
            if (nDepth == 0 && nStart >= 0)
            {
                arrResult.push_back(strContent.Mid(nStart, i - nStart + 1));
                nStart = -1;
            }
        }
    }
    return arrResult;
}

CString ArtifactStore::ReadFileAsWideString(const CString& strPath)
{
    FILE* pFile = nullptr;
    if (_wfopen_s(&pFile, strPath, L"rb") != 0 || !pFile)
        return L"";

    fseek(pFile, 0, SEEK_END);
    long nSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (nSize <= 0) { fclose(pFile); return L""; }

    std::string bytes(nSize, '\0');
    fread(&bytes[0], 1, nSize, pFile);
    fclose(pFile);

    if (bytes.size() >= 3 &&
        (unsigned char)bytes[0] == 0xEF &&
        (unsigned char)bytes[1] == 0xBB &&
        (unsigned char)bytes[2] == 0xBF)
    {
        bytes = bytes.substr(3);
    }

    int nChars = MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), (int)bytes.size(), nullptr, 0);
    if (nChars <= 0) return L"";

    CString strResult;
    MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), (int)bytes.size(),
        strResult.GetBuffer(nChars), nChars);
    strResult.ReleaseBuffer(nChars);
    return strResult;
}

BOOL ArtifactStore::WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError)
{
    int nBytes = WideCharToMultiByte(CP_UTF8, 0, strContent, -1, nullptr, 0, nullptr, nullptr);
    if (nBytes <= 0) { strError = L"UTF-8 변환 실패"; return FALSE; }

    std::string bytes(nBytes - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strContent, -1, &bytes[0], nBytes, nullptr, nullptr);

    FILE* pFile = nullptr;
    if (_wfopen_s(&pFile, strPath, L"wb") != 0 || !pFile)
    {
        strError = L"파일 쓰기 실패: " + strPath;
        return FALSE;
    }

    fwrite(bytes.c_str(), 1, bytes.size(), pFile);
    fclose(pFile);
    return TRUE;
}
