#pragma once
#include "pch.h"
#include "app/host/BridgeDispatcher.h"
#include "app/domain/model/DataTable.h"
#include "app/domain/model/TransformStep.h"

class TransformBridgeHandler
{
public:
    TransformBridgeHandler();

    void RegisterHandlers(BridgeDispatcher& dispatcher, DataTable* pSharedTable);

private:
    CString HandleApplySteps(const BridgeMessage& msg);

    BOOL ParseSteps(const CString& strPayloadJson, std::vector<TransformStep>& arrSteps, CString& strError);
    CString ExtractArrayContent(const CString& strJson, const CString& strKey) const;
    std::vector<CString> SplitJsonObjects(const CString& strArrayContent) const;

    CString SerializeTableToJson(const DataTable& table) const;

    DataTable* m_pSharedTable;
};
