#include "pch.h"
#include "app/application/services/SchedulerService.h"
#include "app/application/SageApp.h"
#include "Define.h"
#include <fstream>
#include <sstream>

SchedulerService::SchedulerService()
{
}

void SchedulerService::LoadFromFile()
{
    CString strPath = sageMgr.GetUserDataDir() + L"\\" + SCHEDULER_FILE_NAME;
    std::string strFilePath = WideToUtf8(strPath);

    std::ifstream file(strFilePath);
    if (!file.is_open())
        return;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string strContent = ss.str();

    m_arrJobs.clear();

    size_t nJobsPos = strContent.find("\"jobs\"");
    if (nJobsPos == std::string::npos)
        return;

    size_t nArrayStart = strContent.find('[', nJobsPos);
    if (nArrayStart == std::string::npos)
        return;

    size_t nArrayEnd = strContent.rfind(']');
    if (nArrayEnd == std::string::npos || nArrayEnd <= nArrayStart)
        return;

    std::string strArray = strContent.substr(nArrayStart + 1, nArrayEnd - nArrayStart - 1);

    size_t nPos = 0;
    while (nPos < strArray.size())
    {
        size_t nObjStart = strArray.find('{', nPos);
        if (nObjStart == std::string::npos)
            break;

        int nDepth = 1;
        size_t nObjEnd = nObjStart + 1;
        while (nObjEnd < strArray.size() && nDepth > 0)
        {
            if (strArray[nObjEnd] == '{') ++nDepth;
            else if (strArray[nObjEnd] == '}') --nDepth;
            ++nObjEnd;
        }
        --nObjEnd;

        std::string strObj = strArray.substr(nObjStart + 1, nObjEnd - nObjStart - 1);

        ScheduledJob job;
        job.m_strJobId        = ExtractStringField(strObj, "jobId");
        job.m_strWorkflowId   = ExtractStringField(strObj, "workflowId");
        job.m_strWorkflowName = ExtractStringField(strObj, "workflowName");
        job.m_strTime         = ExtractStringField(strObj, "time");
        job.m_strNextRunAt    = ExtractStringField(strObj, "nextRunAt");
        job.m_bEnabled        = ExtractBoolField(strObj, "enabled");
        job.m_strCreatedAt    = ExtractStringField(strObj, "createdAt");

        if (!job.m_strJobId.IsEmpty() && !job.m_strWorkflowId.IsEmpty())
            m_arrJobs.push_back(job);

        nPos = nObjEnd + 1;
    }
}

void SchedulerService::SaveToFile() const
{
    CString strPath = sageMgr.GetUserDataDir() + L"\\" + SCHEDULER_FILE_NAME;
    std::string strFilePath = WideToUtf8(strPath);

    std::ofstream file(strFilePath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return;

    file << "{\n  \"jobs\": [\n";
    for (int i = 0; i < (int)m_arrJobs.size(); ++i)
    {
        const ScheduledJob& job = m_arrJobs[i];
        file << "    {\n";
        file << "      \"jobId\": \""       << WideToUtf8(job.m_strJobId)        << "\",\n";
        file << "      \"workflowId\": \""  << WideToUtf8(job.m_strWorkflowId)   << "\",\n";
        file << "      \"workflowName\": \"" << WideToUtf8(job.m_strWorkflowName) << "\",\n";
        file << "      \"time\": \""        << WideToUtf8(job.m_strTime)         << "\",\n";
        file << "      \"nextRunAt\": \""   << WideToUtf8(job.m_strNextRunAt)    << "\",\n";
        file << "      \"enabled\": "       << (job.m_bEnabled ? "true" : "false") << ",\n";
        file << "      \"createdAt\": \""   << WideToUtf8(job.m_strCreatedAt)    << "\"\n";
        file << "    }";
        if (i < (int)m_arrJobs.size() - 1)
            file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
}

void SchedulerService::AddJob(const CString& strWorkflowId, const CString& strWorkflowName,
                               const CString& strTime, CString& strJobIdOut)
{
    ScheduledJob job;
    job.m_strJobId        = GenerateJobId();
    job.m_strWorkflowId   = strWorkflowId;
    job.m_strWorkflowName = strWorkflowName;
    job.m_strTime         = strTime;
    job.m_strNextRunAt    = CalcNextRunAt(strTime);
    job.m_bEnabled        = TRUE;

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    job.m_strCreatedAt.Format(L"%04d-%02d-%02d %02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

    m_arrJobs.push_back(job);
    strJobIdOut = job.m_strJobId;
    SaveToFile();
}

void SchedulerService::RemoveJob(const CString& strJobId)
{
    for (int i = 0; i < (int)m_arrJobs.size(); ++i)
    {
        if (m_arrJobs[i].m_strJobId == strJobId)
        {
            m_arrJobs.erase(m_arrJobs.begin() + i);
            break;
        }
    }
    SaveToFile();
}

void SchedulerService::ToggleJob(const CString& strJobId, BOOL bEnabled)
{
    for (ScheduledJob& job : m_arrJobs)
    {
        if (job.m_strJobId == strJobId)
        {
            job.m_bEnabled = bEnabled;
            break;
        }
    }
    SaveToFile();
}

void SchedulerService::GetJobs(std::vector<ScheduledJob>& arrJobs) const
{
    arrJobs = m_arrJobs;
}

void SchedulerService::GetDueJobs(std::vector<ScheduledJob>& arrDue)
{
    for (ScheduledJob& job : m_arrJobs)
    {
        if (!job.m_bEnabled)
            continue;

        if (IsJobDue(job))
        {
            arrDue.push_back(job);
            UpdateNextRunAt(job);
        }
    }

    if (!arrDue.empty())
        SaveToFile();
}

CString SchedulerService::GenerateJobId() const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);
    CString strId;
    strId.Format(L"SCH-%04d%02d%02d-%02d%02d-%04d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wMilliseconds);
    return strId;
}

CString SchedulerService::CalcNextRunAt(const CString& strTime) const
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);

    CString strNowTime;
    strNowTime.Format(L"%02d:%02d", st.wHour, st.wMinute);

    CString strDate;

    if (strNowTime >= strTime)
    {
        FILETIME ft = {};
        SystemTimeToFileTime(&st, &ft);

        ULARGE_INTEGER uli;
        uli.LowPart  = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uli.QuadPart += (ULONGLONG)10000000 * 60 * 60 * 24;

        ft.dwLowDateTime  = uli.LowPart;
        ft.dwHighDateTime = uli.HighPart;

        SYSTEMTIME stTomorrow = {};
        FileTimeToSystemTime(&ft, &stTomorrow);
        strDate.Format(L"%04d-%02d-%02d", stTomorrow.wYear, stTomorrow.wMonth, stTomorrow.wDay);
    }
    else
    {
        strDate.Format(L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    }

    return strDate + L" " + strTime;
}

BOOL SchedulerService::IsJobDue(const ScheduledJob& job) const
{
    if (job.m_strNextRunAt.IsEmpty())
        return FALSE;

    SYSTEMTIME st = {};
    GetLocalTime(&st);

    CString strNow;
    strNow.Format(L"%04d-%02d-%02d %02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

    return (strNow >= job.m_strNextRunAt) ? TRUE : FALSE;
}

void SchedulerService::UpdateNextRunAt(ScheduledJob& job)
{
    SYSTEMTIME st = {};
    GetLocalTime(&st);

    FILETIME ft = {};
    SystemTimeToFileTime(&st, &ft);

    ULARGE_INTEGER uli;
    uli.LowPart  = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uli.QuadPart += (ULONGLONG)10000000 * 60 * 60 * 24;

    ft.dwLowDateTime  = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;

    SYSTEMTIME stTomorrow = {};
    FileTimeToSystemTime(&ft, &stTomorrow);

    job.m_strNextRunAt.Format(L"%04d-%02d-%02d %s",
        stTomorrow.wYear, stTomorrow.wMonth, stTomorrow.wDay,
        (LPCWSTR)job.m_strTime);
}

CString SchedulerService::ExtractStringField(const std::string& strObj, const std::string& strKey) const
{
    std::string token = "\"" + strKey + "\"";
    size_t nKeyPos = strObj.find(token);
    if (nKeyPos == std::string::npos)
        return L"";

    size_t nColon = strObj.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return L"";

    size_t nOpen = strObj.find('"', nColon + 1);
    if (nOpen == std::string::npos)
        return L"";

    size_t nClose = strObj.find('"', nOpen + 1);
    if (nClose == std::string::npos)
        return L"";

    return Utf8ToWide(strObj.substr(nOpen + 1, nClose - nOpen - 1));
}

BOOL SchedulerService::ExtractBoolField(const std::string& strObj, const std::string& strKey) const
{
    std::string token = "\"" + strKey + "\"";
    size_t nKeyPos = strObj.find(token);
    if (nKeyPos == std::string::npos)
        return FALSE;

    size_t nColon = strObj.find(':', nKeyPos + token.size());
    if (nColon == std::string::npos)
        return FALSE;

    size_t nStart = nColon + 1;
    while (nStart < strObj.size() && strObj[nStart] == ' ')
        ++nStart;

    return (strObj.substr(nStart, 4) == "true") ? TRUE : FALSE;
}
