#pragma once
#include "pch.h"
#include "app/domain/model/ApiConnector.h"
#include "app/domain/model/ApiCallAction.h"
#include <vector>
#include <string>

class ApiConnectorService
{
public:
    ApiConnectorService();

    void LoadFromFile();
    void SaveToFile() const;

    void AddConnector(const CString& strName, const CString& strBaseUrl,
                      const CString& strHeadersJson, const CString& strAuthType,
                      const CString& strAuthValue, CString& strIdOut);
    void RemoveConnector(const CString& strId);
    void UpdateConnector(const CString& strId, const CString& strName, const CString& strBaseUrl,
                         const CString& strHeadersJson, const CString& strAuthType,
                         const CString& strAuthValue);
    void GetConnectors(std::vector<ApiConnector>& arrConnectors) const;
    BOOL GetConnector(const CString& strId, ApiConnector& outConnector) const;
    BOOL TestConnector(const CString& strId, CString& strError);
    BOOL BuildAction(const CString& strId, const CString& strUrlSuffix,
                     const CString& strMethod, const CString& strBody,
                     ApiCallAction& outAction, CString& strError) const;

private:
    CString GenerateConnectorId() const;
    CString BuildMergedHeaders(const ApiConnector& conn) const;
    CString InjectJsonPair(const CString& strJson, const CString& strPair) const;
    CString EscapeJsonValue(const CString& str) const;

    CString ExtractStringField(const std::string& strObj, const std::string& strKey) const;
    BOOL    ExtractBoolField(const std::string& strObj, const std::string& strKey) const;

    std::vector<ApiConnector> m_arrConnectors;
};
