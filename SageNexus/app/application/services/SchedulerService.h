#pragma once
#include "pch.h"
#include "app/domain/model/ScheduledJob.h"
#include <vector>
#include <string>

class SchedulerService
{
public:
    SchedulerService();

    void LoadFromFile();
    void SaveToFile() const;

    void AddJob(const CString& strWorkflowId, const CString& strWorkflowName,
                const CString& strTime, CString& strJobIdOut);
    void RemoveJob(const CString& strJobId);
    void ToggleJob(const CString& strJobId, BOOL bEnabled);
    void GetJobs(std::vector<ScheduledJob>& arrJobs) const;
    void GetDueJobs(std::vector<ScheduledJob>& arrDue);

private:
    CString GenerateJobId() const;
    CString CalcNextRunAt(const CString& strTime) const;
    BOOL    IsJobDue(const ScheduledJob& job) const;
    void    UpdateNextRunAt(ScheduledJob& job);

    CString ExtractStringField(const std::string& strObj, const std::string& strKey) const;
    BOOL    ExtractBoolField(const std::string& strObj, const std::string& strKey) const;

    std::vector<ScheduledJob> m_arrJobs;
};
