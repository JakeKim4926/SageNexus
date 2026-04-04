#include "pch.h"
#include "app/infrastructure/workflow/WorkflowStore.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <string>

BOOL WorkflowStore::SaveWorkflow(WorkflowDefinition& workflow, CString& strError)
{
    if (!EnsureDataDirectory(strError))
        return FALSE;

    std::vector<WorkflowDefinition> arrWorkflows;
    CString strFilePath = BuildFilePath();
    CString strExisting = ReadFileAsWideString(strFilePath);
    if (!strExisting.IsEmpty())
    {
        CString strParseError;
        ParseWorkflows(strExisting, arrWorkflows, strParseError);
    }

    CString strNow = GetCurrentTimestamp();

    if (workflow.m_strId.IsEmpty())
    {
        workflow.m_strId        = GenerateId();
        workflow.m_strCreatedAt = strNow;
        workflow.m_strUpdatedAt = strNow;
        arrWorkflows.push_back(workflow);
    }
    else
    {
        workflow.m_strUpdatedAt = strNow;
        BOOL bFound = FALSE;
        for (int i = 0; i < (int)arrWorkflows.size(); ++i)
        {
            if (arrWorkflows[i].m_strId == workflow.m_strId)
            {
                arrWorkflows[i] = workflow;
                bFound = TRUE;
                break;
            }
        }
        if (!bFound)
            arrWorkflows.push_back(workflow);
    }

    CString strJson = SerializeWorkflows(arrWorkflows);
    return WriteFileAsUtf8(strFilePath, strJson, strError);
}

BOOL WorkflowStore::LoadWorkflows(std::vector<WorkflowDefinition>& arrWorkflows, CString& strError) const
{
    CString strFilePath = BuildFilePath();
    CString strJson     = ReadFileAsWideString(strFilePath);
    if (strJson.IsEmpty())
        return TRUE;

    return ParseWorkflows(strJson, arrWorkflows, strError);
}

BOOL WorkflowStore::DeleteWorkflow(const CString& strId, CString& strError)
{
    if (!EnsureDataDirectory(strError))
        return FALSE;

    CString strFilePath = BuildFilePath();
    CString strExisting = ReadFileAsWideString(strFilePath);

    std::vector<WorkflowDefinition> arrWorkflows;
    if (!strExisting.IsEmpty())
    {
        CString strParseError;
        ParseWorkflows(strExisting, arrWorkflows, strParseError);
    }

    std::vector<WorkflowDefinition> arrFiltered;
    for (const WorkflowDefinition& wf : arrWorkflows)
    {
        if (wf.m_strId != strId)
            arrFiltered.push_back(wf);
    }

    CString strJson = SerializeWorkflows(arrFiltered);
    return WriteFileAsUtf8(strFilePath, strJson, strError);
}

CString WorkflowStore::BuildFilePath() const
{
    return sageMgr.GetAppDir() + L"\\" + DATA_DIR_NAME + L"\\workflows.json";
}

BOOL WorkflowStore::EnsureDataDirectory(CString& strError) const
{
    CString strDataDir = sageMgr.GetAppDir() + L"\\" + DATA_DIR_NAME;
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

CString WorkflowStore::GenerateId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"wf-%04d%02d%02d%02d%02d%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strId;
}

CString WorkflowStore::GetCurrentTimestamp() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strTs;
    strTs.Format(L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    return strTs;
}

CString WorkflowStore::SerializeWorkflows(const std::vector<WorkflowDefinition>& arrWorkflows) const
{
    CString strJson = L"[";
    for (int i = 0; i < (int)arrWorkflows.size(); ++i)
    {
        if (i > 0) strJson += L",";
        strJson += SerializeWorkflow(arrWorkflows[i]);
    }
    strJson += L"]";
    return strJson;
}

CString WorkflowStore::SerializeWorkflow(const WorkflowDefinition& workflow) const
{
    CString strJson = L"{";
    strJson += L"\"id\":\""          + EscapeJsonString(workflow.m_strId)          + L"\",";
    strJson += L"\"name\":\""        + EscapeJsonString(workflow.m_strName)        + L"\",";
    strJson += L"\"description\":\"" + EscapeJsonString(workflow.m_strDescription) + L"\",";
    strJson += L"\"createdAt\":\""   + EscapeJsonString(workflow.m_strCreatedAt)   + L"\",";
    strJson += L"\"updatedAt\":\""   + EscapeJsonString(workflow.m_strUpdatedAt)   + L"\",";

    strJson += L"\"steps\":[";
    for (int i = 0; i < (int)workflow.m_arrSteps.size(); ++i)
    {
        if (i > 0) strJson += L",";
        strJson += SerializeStep(workflow.m_arrSteps[i]);
    }
    strJson += L"]}";
    return strJson;
}

CString WorkflowStore::SerializeStep(const WorkflowStep& step) const
{
    CString strJson = L"{";
    strJson += L"\"id\":\""       + EscapeJsonString(step.m_strId)       + L"\",";
    strJson += L"\"stepType\":\"" + EscapeJsonString(step.m_strStepType) + L"\",";
    strJson += L"\"name\":\""     + EscapeJsonString(step.m_strName)     + L"\",";
    strJson += L"\"configJson\":\"" + EscapeJsonString(step.m_strConfigJson) + L"\"";
    strJson += L"}";
    return strJson;
}

BOOL WorkflowStore::ParseWorkflows(const CString& strJson, std::vector<WorkflowDefinition>& arrWorkflows, CString& strError) const
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
        arrWorkflows.push_back(ParseWorkflow(strObj));
    }
    return TRUE;
}

WorkflowDefinition WorkflowStore::ParseWorkflow(const CString& strObj) const
{
    WorkflowDefinition wf;
    wf.m_strId          = UnescapeJsonString(ExtractStringField(strObj, L"id"));
    wf.m_strName        = UnescapeJsonString(ExtractStringField(strObj, L"name"));
    wf.m_strDescription = UnescapeJsonString(ExtractStringField(strObj, L"description"));
    wf.m_strCreatedAt   = UnescapeJsonString(ExtractStringField(strObj, L"createdAt"));
    wf.m_strUpdatedAt   = UnescapeJsonString(ExtractStringField(strObj, L"updatedAt"));

    CString strStepsJson = ExtractArrayField(strObj, L"steps");
    if (!strStepsJson.IsEmpty())
        wf.m_arrSteps = ParseSteps(strStepsJson);

    return wf;
}

std::vector<WorkflowStep> WorkflowStore::ParseSteps(const CString& strStepsJson) const
{
    std::vector<WorkflowStep> arrSteps;

    int nStart = strStepsJson.Find(L'[');
    int nEnd   = strStepsJson.ReverseFind(L']');
    if (nStart < 0 || nEnd < 0 || nEnd <= nStart)
        return arrSteps;

    CString strContent = strStepsJson.Mid(nStart + 1, nEnd - nStart - 1);
    std::vector<CString> arrObjects = SplitJsonObjects(strContent);

    for (const CString& strObj : arrObjects)
    {
        if (strObj.IsEmpty())
            continue;
        arrSteps.push_back(ParseStep(strObj));
    }
    return arrSteps;
}

WorkflowStep WorkflowStore::ParseStep(const CString& strObj) const
{
    WorkflowStep step;
    step.m_strId         = UnescapeJsonString(ExtractStringField(strObj, L"id"));
    step.m_strStepType   = UnescapeJsonString(ExtractStringField(strObj, L"stepType"));
    step.m_strName       = UnescapeJsonString(ExtractStringField(strObj, L"name"));
    step.m_strConfigJson = UnescapeJsonString(ExtractStringField(strObj, L"configJson"));
    return step;
}

CString WorkflowStore::EscapeJsonString(const CString& str) const
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

CString WorkflowStore::UnescapeJsonString(const CString& str) const
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

CString WorkflowStore::ExtractStringField(const CString& strJson, const CString& strKey) const
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

CString WorkflowStore::ExtractArrayField(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0) return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0) return L"";

    int nValueStart = nColon + 1;
    while (nValueStart < strJson.GetLength() && strJson[nValueStart] == L' ')
        ++nValueStart;

    if (nValueStart >= strJson.GetLength() || strJson[nValueStart] != L'[')
        return L"";

    int nDepth = 0;
    int nEnd   = nValueStart;
    for (; nEnd < strJson.GetLength(); ++nEnd)
    {
        if (strJson[nEnd] == L'[') ++nDepth;
        else if (strJson[nEnd] == L']')
        {
            --nDepth;
            if (nDepth == 0) break;
        }
    }

    return strJson.Mid(nValueStart, nEnd - nValueStart + 1);
}

std::vector<CString> WorkflowStore::SplitJsonObjects(const CString& strContent) const
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

CString WorkflowStore::ReadFileAsWideString(const CString& strPath)
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

BOOL WorkflowStore::WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError)
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
