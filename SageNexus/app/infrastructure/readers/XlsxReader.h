#pragma once
#include "pch.h"
#include "app/domain/interfaces/IDataReader.h"
#include "app/domain/model/DataTable.h"

class XlsxReader : public IDataReader
{
public:
    BOOL Read(const CString& strFilePath, DataTable& outTable, CString& strError) override;

private:
    BOOL ExtractZip(const CString& strZipPath, const CString& strDestDir, CString& strError) const;
    BOOL ParseSharedStrings(const CString& strPath, std::vector<CString>& arrStrings) const;
    BOOL ParseStyles(const CString& strPath, std::vector<BOOL>& arrIsDateXf) const;
    BOOL ParseSheet(const CString& strPath, const std::vector<CString>& arrShared,
                    const std::vector<BOOL>& arrIsDateXf,
                    DataTable& outTable, CString& strError) const;
    void CleanupDir(const CString& strDir) const;

    CString              ReadXmlFile(const CString& strPath) const;
    std::vector<CString> FindRows(const CString& strXml) const;
    std::vector<CString> FindCells(const CString& strRowXml) const;
    CString              GetAttribute(const CString& strElement, const CString& strAttr) const;
    CString              GetTagContent(const CString& strXml, const CString& strTag) const;
    CString              GetAllTagContent(const CString& strXml, const CString& strTag) const;
    CString              UnescapeXml(const CString& str) const;
    int                  ColRefToIndex(const CString& strCellRef) const;
    CString              GetCellValue(const CString& strCellXml,
                                      const std::vector<CString>& arrShared,
                                      BOOL bIsDateFmt) const;
    BOOL                 IsDateNumFmtId(int nId) const;
    BOOL                 IsDateFormatCode(const CString& strCode) const;
    CString              SerialToDate(int nSerial) const;
};
