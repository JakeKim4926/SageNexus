#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/application/services/JobQueueService.h"

class JobQueueBridgeHandler
{
public:
    JobQueueBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, HWND hMainWnd);
    const CString& GetCurrentStepName() const;

private:
    CString HandleEnqueue(const BridgeMessage& msg);
    CString HandleGetQueue(const BridgeMessage& msg);
    CString HandleCancelJob(const BridgeMessage& msg);
    CString HandleCancelAll(const BridgeMessage& msg);
    CString HandleRetryJob(const BridgeMessage& msg);

    CString SerializeJob(const ExecutionJob& job) const;
    CString JobStatusToString(JobStatus eStatus) const;
    CString ExtractPayloadString(const CString& strJson, const CString& strKey) const;
    CString EscapeJson(const CString& str) const;

    JobQueueService m_service;
    HWND            m_hMainWnd;
};
