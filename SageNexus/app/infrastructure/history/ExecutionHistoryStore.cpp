#include "pch.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "app/application/SageApp.h"
#include "Define.h"

ExecutionHistoryStore::ExecutionHistoryStore()
{
}

BOOL ExecutionHistoryStore::SaveRecord(ExecutionRecord& record, CString& strError)
{
    record.m_strRunId     = GenerateRunId();
    record.m_strTimestamp = GetCurrentTimestamp();

    if (!EnsureDataDirectory(strError))
        return FALSE;

    CString strFilePath = BuildFilePath();

    std::vector<ExecutionRecord> arrRecords;
    CString strExisting = ReadFileAsWideString(strFilePath);
    if (!strExisting.IsEmpty())
    {
        CString strParseError;
        ParseRecords(strExisting, arrRecords, strParseError);
    }

    arrRecords.push_back(record);

    CString strJson = SerializeRecords(arrRecords);
    return WriteFileAsUtf8(strFilePath, strJson, strError);
}

BOOL ExecutionHistoryStore::LoadRecords(std::vector<ExecutionRecord>& arrRecords, CString& strError) const
{
    CString strFilePath = BuildFilePath();
    CString strJson = ReadFileAsWideString(strFilePath);
    if (strJson.IsEmpty())
        return TRUE;

    return ParseRecords(strJson, arrRecords, strError);
}

CString ExecutionHistoryStore::BuildFilePath() const
{
    return sageMgr.GetDataDir() + L"\\execution_history.json";
}

BOOL ExecutionHistoryStore::EnsureDataDirectory(CString& strError) const
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

CString ExecutionHistoryStore::GenerateRunId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"run-%04d%02d%02d%02d%02d%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strId;
}

CString ExecutionHistoryStore::GetCurrentTimestamp() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strTs;
    strTs.Format(L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strTs;
}

CString ExecutionHistoryStore::SerializeRecords(const std::vector<ExecutionRecord>& arrRecords) const
{
    CString strJson = L"[";
    for (int i = 0; i < (int)arrRecords.size(); ++i)
    {
        if (i > 0) strJson += L",";
        strJson += SerializeRecord(arrRecords[i]);
    }
    strJson += L"]";
    return strJson;
}

CString ExecutionHistoryStore::SerializeRecord(const ExecutionRecord& record) const
{
    wchar_t buf[32];
    CString strJson = L"{";
    strJson += L"\"runId\":\"" + EscapeJsonString(record.m_strRunId) + L"\",";
    strJson += L"\"timestamp\":\"" + EscapeJsonString(record.m_strTimestamp) + L"\",";
    strJson += L"\"operationType\":\"" + EscapeJsonString(record.m_strOperationType) + L"\",";
    strJson += L"\"sourceName\":\"" + EscapeJsonString(record.m_strSourceName) + L"\",";
    swprintf_s(buf, 32, L"%d", record.m_nRowCount);
    strJson += L"\"rowCount\":"; strJson += buf; strJson += L",";
    swprintf_s(buf, 32, L"%d", record.m_nColumnCount);
    strJson += L"\"columnCount\":"; strJson += buf; strJson += L",";
    strJson += L"\"outputPath\":\"" + EscapeJsonString(record.m_strOutputPath) + L"\",";
    strJson += L"\"success\":";
    strJson += (record.m_bSuccess ? L"true" : L"false");
    strJson += L",";
    strJson += L"\"errorMessage\":\"" + EscapeJsonString(record.m_strErrorMessage) + L"\"";
    strJson += L"}";
    return strJson;
}

BOOL ExecutionHistoryStore::ParseRecords(const CString& strJson, std::vector<ExecutionRecord>& arrRecords, CString& strError) const
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

        ExecutionRecord rec;
        rec.m_strRunId          = UnescapeJsonString(ExtractStringField(strObj, L"runId"));
        rec.m_strTimestamp      = UnescapeJsonString(ExtractStringField(strObj, L"timestamp"));
        rec.m_strOperationType  = UnescapeJsonString(ExtractStringField(strObj, L"operationType"));
        rec.m_strSourceName     = UnescapeJsonString(ExtractStringField(strObj, L"sourceName"));
        rec.m_nRowCount         = ExtractIntField(strObj, L"rowCount");
        rec.m_nColumnCount      = ExtractIntField(strObj, L"columnCount");
        rec.m_strOutputPath     = UnescapeJsonString(ExtractStringField(strObj, L"outputPath"));
        rec.m_bSuccess          = ExtractBoolField(strObj, L"success");
        rec.m_strErrorMessage   = UnescapeJsonString(ExtractStringField(strObj, L"errorMessage"));
        arrRecords.push_back(rec);
    }
    return TRUE;
}

std::vector<CString> ExecutionHistoryStore::SplitJsonObjects(const CString& strContent) const
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
                if (strContent[i] == L'\\')
                    ++i;
                else if (strContent[i] == L'"')
                    break;
                ++i;
            }
            continue;
        }

        if (ch == L'{')
        {
            if (nDepth == 0)
                nStart = i;
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

CString ExecutionHistoryStore::EscapeJsonString(const CString& str) const
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

CString ExecutionHistoryStore::UnescapeJsonString(const CString& str) const
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

CString ExecutionHistoryStore::ExtractStringField(const CString& strJson, const CString& strKey) const
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
        if (strJson[nEnd] == L'\\')
        {
            nEnd += 2;
            continue;
        }
        if (strJson[nEnd] == L'"')
            break;
        ++nEnd;
    }

    if (nEnd >= strJson.GetLength())
        return L"";

    return strJson.Mid(nValueStart + 1, nEnd - nValueStart - 1);
}

int ExecutionHistoryStore::ExtractIntField(const CString& strJson, const CString& strKey) const
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

BOOL ExecutionHistoryStore::ExtractBoolField(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0) return FALSE;

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0) return FALSE;

    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    return (strJson.Mid(nValueStart, 4) == L"true") ? TRUE : FALSE;
}

CString ExecutionHistoryStore::ReadFileAsWideString(const CString& strPath)
{
    FILE* pFile = nullptr;
    if (_wfopen_s(&pFile, strPath, L"rb") != 0 || !pFile)
        return L"";

    fseek(pFile, 0, SEEK_END);
    long nSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (nSize <= 0)
    {
        fclose(pFile);
        return L"";
    }

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

BOOL ExecutionHistoryStore::WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError)
{
    int nBytes = WideCharToMultiByte(CP_UTF8, 0, strContent, -1, nullptr, 0, nullptr, nullptr);
    if (nBytes <= 0)
    {
        strError = L"UTF-8 변환 실패";
        return FALSE;
    }

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
