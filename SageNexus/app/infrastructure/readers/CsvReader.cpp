#include "pch.h"
#include "app/infrastructure/readers/CsvReader.h"

CsvReader::CsvReader()
{
}

CsvReader::~CsvReader()
{
}

BOOL CsvReader::Read(const CString& strFilePath, DataTable& outTable, CString& strError)
{
    std::ifstream file(strFilePath, std::ios::binary);
    if (!file.is_open())
    {
        strError = L"파일을 열 수 없습니다: " + strFilePath;
        return FALSE;
    }

    std::string strRaw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (strRaw.empty())
    {
        strError = L"파일이 비어 있습니다.";
        return FALSE;
    }

    std::wstring strContent;
    if (strRaw.size() >= 3 &&
        (unsigned char)strRaw[0] == 0xEF &&
        (unsigned char)strRaw[1] == 0xBB &&
        (unsigned char)strRaw[2] == 0xBF)
    {
        strContent = ConvertToWide(strRaw.substr(3), CP_UTF8);
    }
    else
    {
        strContent = ConvertToWide(strRaw, CP_UTF8);
        if (strContent.empty())
            strContent = ConvertToWide(strRaw, CP_ACP);
    }

    std::vector<std::wstring> arrLines = SplitLines(strContent);
    if (arrLines.empty())
    {
        strError = L"CSV 파일에서 읽을 행이 없습니다.";
        return FALSE;
    }

    std::vector<CString> arrHeaders = ParseRow(arrLines[0]);
    if (arrHeaders.empty())
    {
        strError = L"CSV 헤더 행을 읽을 수 없습니다.";
        return FALSE;
    }

    for (int i = 0; i < (int)arrHeaders.size(); ++i)
    {
        DataColumn col;
        col.m_strInternalName  = arrHeaders[i];
        col.m_strSourceName    = arrHeaders[i];
        col.m_strDisplayNameKo = arrHeaders[i];
        col.m_strDisplayNameEn = arrHeaders[i];
        col.m_strDataType      = L"string";
        outTable.AddColumn(col);
    }

    int nColCount = (int)arrHeaders.size();
    for (int i = 1; i < (int)arrLines.size(); ++i)
    {
        if (arrLines[i].empty())
            continue;

        std::vector<CString> arrCells = ParseRow(arrLines[i]);

        DataRow row;
        for (int j = 0; j < nColCount; ++j)
        {
            if (j < (int)arrCells.size())
                row.m_arrCells.push_back(arrCells[j]);
            else
                row.m_arrCells.push_back(L"");
        }
        outTable.AddRow(row);
    }

    return TRUE;
}

std::wstring CsvReader::ConvertToWide(const std::string& strBytes, UINT nCodePage)
{
    if (strBytes.empty())
        return L"";

    int nLen = MultiByteToWideChar(nCodePage, 0, strBytes.c_str(), (int)strBytes.size(), nullptr, 0);
    if (nLen <= 0)
        return L"";

    std::wstring strWide(nLen, L'\0');
    MultiByteToWideChar(nCodePage, 0, strBytes.c_str(), (int)strBytes.size(), &strWide[0], nLen);
    return strWide;
}

std::vector<std::wstring> CsvReader::SplitLines(const std::wstring& strContent)
{
    std::vector<std::wstring> arrLines;
    std::wistringstream stream(strContent);
    std::wstring strLine;

    while (std::getline(stream, strLine))
    {
        if (!strLine.empty() && strLine.back() == L'\r')
            strLine.pop_back();
        arrLines.push_back(strLine);
    }

    return arrLines;
}

std::vector<CString> CsvReader::ParseRow(const std::wstring& strLine)
{
    std::vector<CString> arrFields;
    std::wstring strField;
    BOOL bInQuotes = FALSE;

    for (int i = 0; i < (int)strLine.size(); ++i)
    {
        wchar_t ch = strLine[i];

        if (bInQuotes)
        {
            if (ch == L'"')
            {
                if (i + 1 < (int)strLine.size() && strLine[i + 1] == L'"')
                {
                    strField += L'"';
                    ++i;
                }
                else
                {
                    bInQuotes = FALSE;
                }
            }
            else
            {
                strField += ch;
            }
        }
        else
        {
            if (ch == L'"')
            {
                bInQuotes = TRUE;
            }
            else if (ch == L',')
            {
                arrFields.push_back(CString(strField.c_str()));
                strField.clear();
            }
            else if (ch == L'\r')
            {
            }
            else
            {
                strField += ch;
            }
        }
    }

    arrFields.push_back(CString(strField.c_str()));
    return arrFields;
}
