#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/SchedulerService.h"

class SchedulerBridgeHandler
{
public:
    SchedulerBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher);
    void GetDueJobs(std::vector<ScheduledJob>& arrDue);

private:
    CString HandleGetJobs(const BridgeMessage& msg);
    CString HandleAddJob(const BridgeMessage& msg);
    CString HandleRemoveJob(const BridgeMessage& msg);
    CString HandleToggleJob(const BridgeMessage& msg);

    CString SerializeJob(const ScheduledJob& job) const;

    SchedulerService m_service;
};
