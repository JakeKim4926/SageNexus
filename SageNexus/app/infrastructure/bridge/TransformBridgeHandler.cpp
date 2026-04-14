#include "pch.h"
#include "app/infrastructure/bridge/TransformBridgeHandler.h"
#include "app/application/services/TransformService.h"
#include "app/infrastructure/history/ExecutionHistoryStore.h"
#include "Define.h"

TransformBridgeHandler::TransformBridgeHandler()
    : m_pSharedTable(nullptr)
{
}

void TransformBridgeHandler::RegisterHandlers(BridgeDispatcher& dispatcher, DataTable* pSharedTable)
{
    m_pSharedTable = pSharedTable;

    dispatcher.RegisterHandler(L"data.transform", L"applySteps",
        [this](const BridgeMessage& msg) -> CString
        {
            return HandleApplySteps(msg);
        });
}

CString TransformBridgeHandler::HandleApplySteps(const BridgeMessage& msg)
{
    if (!m_pSharedTable || m_pSharedTable->IsEmpty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_TF_001\",\"message\":\"변환할 데이터가 없습니다.\"}}";
    }

    std::vector<TransformStep> arrSteps;
    CString strParseError;
    if (!ParseSteps(msg.m_strPayloadJson, arrSteps, strParseError))
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_TF_002\",\"message\":\"" +
               JsonEscapeString(strParseError) + L"\"}}";
    }

    if (arrSteps.empty())
    {
        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_TF_003\",\"message\":\"변환 단계가 없습니다.\"}}";
    }

    TransformService service;
    CString strError;
    if (!service.ApplySteps(*m_pSharedTable, arrSteps, strError))
    {
        ExecutionHistoryStore historyStore;
        ExecutionRecord histRecord;
        histRecord.m_strOperationType = L"transform";
        histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
        histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
        histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
        histRecord.m_bSuccess         = FALSE;
        histRecord.m_strErrorMessage  = strError;
        CString strHistError;
        historyStore.SaveRecord(histRecord, strHistError);

        return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
               L"\",\"success\":false,\"payload\":null,"
               L"\"error\":{\"code\":\"SNX_TF_004\",\"message\":\"" +
               JsonEscapeString(strError) + L"\"}}";
    }

    ExecutionHistoryStore historyStore;
    ExecutionRecord histRecord;
    histRecord.m_strOperationType = L"transform";
    histRecord.m_strSourceName    = m_pSharedTable->GetSourceName();
    histRecord.m_nRowCount        = m_pSharedTable->GetRowCount();
    histRecord.m_nColumnCount     = m_pSharedTable->GetColumnCount();
    histRecord.m_bSuccess         = TRUE;
    CString strHistError;
    historyStore.SaveRecord(histRecord, strHistError);

    CString strTableJson = SerializeTableToJson(*m_pSharedTable);
    return L"{\"type\":\"response\",\"requestId\":\"" + msg.m_strRequestId +
           L"\",\"success\":true,\"payload\":" + strTableJson + L"}";
}

BOOL TransformBridgeHandler::ParseSteps(const CString& strPayloadJson, std::vector<TransformStep>& arrSteps, CString& strError)
{
    CString strArrayContent = ExtractArrayContent(strPayloadJson, L"steps");
    if (strArrayContent.IsEmpty())
    {
        strError = L"steps 배열을 찾을 수 없습니다.";
        return FALSE;
    }

    std::vector<CString> arrObjects = SplitJsonObjects(strArrayContent);
    for (const CString& strObj : arrObjects)
    {
        if (strObj.IsEmpty())
            continue;

        TransformStep step;
        step.m_strType    = UnescapeJsonString(ExtractStringField(strObj, L"type"));
        step.m_strColumn  = UnescapeJsonString(ExtractStringField(strObj, L"column"));
        step.m_strParam1  = UnescapeJsonString(ExtractStringField(strObj, L"param1"));
        step.m_strParam2  = UnescapeJsonString(ExtractStringField(strObj, L"param2"));

        if (step.m_strType.IsEmpty())
        {
            strError = L"변환 단계에 type이 없습니다.";
            return FALSE;
        }

        arrSteps.push_back(step);
    }
    return TRUE;
}

CString TransformBridgeHandler::ExtractArrayContent(const CString& strJson, const CString& strKey) const
{
    CString strSearch = L"\"" + strKey + L"\"";
    int nPos = strJson.Find(strSearch);
    if (nPos < 0)
        return L"";

    int nColon = strJson.Find(L':', nPos + strSearch.GetLength());
    if (nColon < 0)
        return L"";

    int nStart = nColon + 1;
    while (nStart < strJson.GetLength() && strJson[nStart] == L' ')
        ++nStart;

    if (nStart >= strJson.GetLength() || strJson[nStart] != L'[')
        return L"";

    int nDepth = 0;
    int nEnd = nStart;
    while (nEnd < strJson.GetLength())
    {
        if (strJson[nEnd] == L'[')
            ++nDepth;
        else if (strJson[nEnd] == L']')
        {
            --nDepth;
            if (nDepth == 0)
                break;
        }
        else if (strJson[nEnd] == L'"')
        {
            ++nEnd;
            while (nEnd < strJson.GetLength())
            {
                if (strJson[nEnd] == L'\\')
                    ++nEnd;
                else if (strJson[nEnd] == L'"')
                    break;
                ++nEnd;
            }
        }
        ++nEnd;
    }

    return strJson.Mid(nStart + 1, nEnd - nStart - 1);
}

CString TransformBridgeHandler::ExtractStringField(const CString& strJson, const CString& strKey) const
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

    return strJson.Mid(nValueStart + 1, nEnd - nValueStart - 1);
}

std::vector<CString> TransformBridgeHandler::SplitJsonObjects(const CString& strArrayContent) const
{
    std::vector<CString> arrResult;
    int nDepth = 0;
    int nStart = -1;

    for (int i = 0; i < strArrayContent.GetLength(); ++i)
    {
        wchar_t ch = strArrayContent[i];

        if (ch == L'"')
        {
            ++i;
            while (i < strArrayContent.GetLength())
            {
                if (strArrayContent[i] == L'\\')
                    ++i;
                else if (strArrayContent[i] == L'"')
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
                arrResult.push_back(strArrayContent.Mid(nStart, i - nStart + 1));
                nStart = -1;
            }
        }
    }
    return arrResult;
}

CString TransformBridgeHandler::SerializeTableToJson(const DataTable& table) const
{
    std::wstring json;
    json.reserve(64 * 1024);
    json += L"{";

    json += L"\"sourceName\":\"";
    json += (LPCWSTR)JsonEscapeString(table.GetSourceName());
    json += L"\",";

    wchar_t buf[32];
    swprintf_s(buf, 32, L"%d", table.GetRowCount());
    json += L"\"rowCount\":";
    json += buf;
    json += L",";

    swprintf_s(buf, 32, L"%d", table.GetColumnCount());
    json += L"\"columnCount\":";
    json += buf;
    json += L",";

    json += L"\"columns\":[";
    for (int i = 0; i < table.GetColumnCount(); ++i)
    {
        if (i > 0) json += L",";
        const DataColumn& col = table.GetColumn(i);
        json += L"{\"internalName\":\"";
        json += (LPCWSTR)JsonEscapeString(col.m_strInternalName);
        json += L"\",\"displayNameKo\":\"";
        CString strDisplayKo = col.m_strDisplayNameKo.IsEmpty() ? col.m_strInternalName : col.m_strDisplayNameKo;
        json += (LPCWSTR)JsonEscapeString(strDisplayKo);
        json += L"\",\"displayNameEn\":\"";
        CString strDisplayEn = col.m_strDisplayNameEn.IsEmpty() ? col.m_strInternalName : col.m_strDisplayNameEn;
        json += (LPCWSTR)JsonEscapeString(strDisplayEn);
        json += L"\"}";
    }
    json += L"],";

    int nMaxRows = table.GetRowCount();
    if (nMaxRows > CSV_MAX_PREVIEW_ROWS)
        nMaxRows = CSV_MAX_PREVIEW_ROWS;

    json += L"\"rows\":[";
    for (int i = 0; i < nMaxRows; ++i)
    {
        if (i > 0) json += L",";
        const DataRow& row = table.GetRow(i);
        json += L"[";
        for (int j = 0; j < (int)row.m_arrCells.size(); ++j)
        {
            if (j > 0) json += L",";
            json += L"\"";
            json += (LPCWSTR)JsonEscapeString(row.m_arrCells[j]);
            json += L"\"";
        }
        json += L"]";
    }
    json += L"]";

    json += L"}";
    return CString(json.c_str());
}
