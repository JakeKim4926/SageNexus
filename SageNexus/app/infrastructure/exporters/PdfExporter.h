#pragma once
#include "pch.h"
#include "app/domain/model/DataTable.h"

class PdfExporter
{
public:
    BOOL Export(const DataTable& table, const CString& strFilePath, const CString& strLang, CString& strError);

private:
    BOOL ConvertHtmlToPdf(const CString& strHtmlPath, const CString& strPdfPath, CString& strError) const;
    CString FindEdgePath() const;
};
