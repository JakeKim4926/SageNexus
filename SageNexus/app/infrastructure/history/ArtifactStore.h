#pragma once
#include "pch.h"
#include "app/domain/model/Artifact.h"
#include <vector>
#include <string>

class ArtifactStore
{
public:
    BOOL SaveArtifact(Artifact& artifact, CString& strError);
    BOOL LoadArtifacts(std::vector<Artifact>& arrArtifacts, CString& strError) const;

private:
    CString BuildFilePath() const;
    BOOL    EnsureDataDirectory(CString& strError) const;

    CString SerializeArtifacts(const std::vector<Artifact>& arrArtifacts) const;
    CString SerializeArtifact(const Artifact& artifact) const;
    BOOL    ParseArtifacts(const CString& strJson, std::vector<Artifact>& arrArtifacts, CString& strError) const;

    CString GenerateId() const;
    CString GetCurrentTimestamp() const;

    CString              EscapeJsonString(const CString& str) const;
    CString              UnescapeJsonString(const CString& str) const;
    CString              ExtractStringField(const CString& strJson, const CString& strKey) const;
    int                  ExtractIntField(const CString& strJson, const CString& strKey) const;
    std::vector<CString> SplitJsonObjects(const CString& strContent) const;

    static CString ReadFileAsWideString(const CString& strPath);
    static BOOL    WriteFileAsUtf8(const CString& strPath, const CString& strContent, CString& strError);
};
