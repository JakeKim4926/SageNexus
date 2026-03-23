#include "pch.h"
#include "app/application/services/ImportService.h"
#include "app/infrastructure/readers/CsvReader.h"
#include "app/infrastructure/readers/XlsxReader.h"

ImportService::ImportService()
{
}

ImportService::~ImportService()
{
}

BOOL ImportService::LoadFromFile(const CString& strFilePath, DataTable& outTable, CString& strError)
{
    if (strFilePath.IsEmpty())
    {
        strError = L"파일 경로가 비어 있습니다.";
        return FALSE;
    }

    if (GetFileAttributes(strFilePath) == INVALID_FILE_ATTRIBUTES)
    {
        strError = L"파일을 찾을 수 없습니다: " + strFilePath;
        return FALSE;
    }

    int nDotPos = strFilePath.ReverseFind(L'.');
    if (nDotPos < 0)
    {
        strError = L"파일 확장자를 확인할 수 없습니다.";
        return FALSE;
    }

    CString strExt = strFilePath.Mid(nDotPos + 1);
    strExt.MakeLower();

    int nBackslashPos = strFilePath.ReverseFind(L'\\');
    int nFwdSlashPos  = strFilePath.ReverseFind(L'/');
    int nSlashPos     = (nBackslashPos > nFwdSlashPos) ? nBackslashPos : nFwdSlashPos;
    CString strFileName = (nSlashPos >= 0) ? strFilePath.Mid(nSlashPos + 1) : strFilePath;

    outTable.Clear();
    outTable.SetSourceName(strFileName);

    if (strExt == L"csv")
    {
        CsvReader reader;
        return reader.Read(strFilePath, outTable, strError);
    }

    if (strExt == L"xlsx")
    {
        XlsxReader reader;
        return reader.Read(strFilePath, outTable, strError);
    }

    strError = L"지원하지 않는 파일 형식입니다: ." + strExt;
    return FALSE;
}
