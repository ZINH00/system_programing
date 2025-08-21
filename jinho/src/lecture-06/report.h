#pragma once
#include <map>
#include <vector>

struct ST_SUMMARY : public core::IFormatterObject
{
    std::tstring strResult;
    std::tstring strDetectName;
    std::tstring strEngineName;
    std::tstring strEngineVersion;
    int          nSeverity = 0;
    std::tstring strSampleName;
    std::tstring strSampleExt;
    double       dTotalElapsedTime = 0.0;
    bool         bReachedFileLoopLimit = false;
    bool         bReachedUrlLoopLimit = false;

    void OnSync(core::IFormatter& formatter)
    {
        formatter.Sync(TEXT("Result"), strResult);
        formatter.Sync(TEXT("DetectName"), strDetectName);
        formatter.Sync(TEXT("EngineName"), strEngineName);
        formatter.Sync(TEXT("EngineVersion"), strEngineVersion);
        formatter.Sync(TEXT("Severity"), nSeverity);
        formatter.Sync(TEXT("SampleName"), strSampleName);
        formatter.Sync(TEXT("SampleExt"), strSampleExt);
        formatter.Sync(TEXT("TotalElapsedTime"), dTotalElapsedTime);
        formatter.Sync(TEXT("ReachedFileLoopLimit"), bReachedFileLoopLimit);
        formatter.Sync(TEXT("ReachedUrlLoopLimit"), bReachedUrlLoopLimit);
    }
};

struct ST_FILETARGET_ITEM : public core::IFormatterObject
{
    std::tstring strTargetID;
    std::tstring strParentID;
    std::tstring strAbsolutePath;
    std::tstring strFileName;
    int          nFileSize = 0;
    std::tstring strFileEXT;
    std::tstring strMD5;
    std::tstring strSHA1;
    std::tstring strSHA256;
    std::tstring strHAS160;
    std::tstring strFileHeaderDump;

    void OnSync(core::IFormatter& formatter)
    {
        formatter.Sync(TEXT("TargetID"), strTargetID);
        formatter.Sync(TEXT("ParentID"), strParentID);
        formatter.Sync(TEXT("AbsolutePath"), strAbsolutePath);
        formatter.Sync(TEXT("FileName"), strFileName);
        formatter.Sync(TEXT("FileSize"), nFileSize);
        formatter.Sync(TEXT("FileEXT"), strFileEXT);
        formatter.Sync(TEXT("MD5"), strMD5);
        formatter.Sync(TEXT("SHA1"), strSHA1);
        formatter.Sync(TEXT("SHA256"), strSHA256);
        formatter.Sync(TEXT("HAS160"), strHAS160);
        formatter.Sync(TEXT("FileHeaderDump"), strFileHeaderDump);
    }
};

struct ST_EVENT : public core::IFormatterObject
{
    std::tstring strEngineName;
    std::tstring strEngineVersion;
    std::tstring strTargetID;
    std::tstring strName;
    std::tstring strAnalysisCode;
    int          nSeverity = 0;
    std::tstring strDesc;
    std::map<std::tstring, std::tstring> mapDescInternational;

    void OnSync(core::IFormatter& formatter)
    {
        formatter.Sync(TEXT("EngineName"), strEngineName);
        formatter.Sync(TEXT("EngineVersion"), strEngineVersion);
        formatter.Sync(TEXT("TargetID"), strTargetID);
        formatter.Sync(TEXT("Name"), strName);
        formatter.Sync(TEXT("AnalysisCode"), strAnalysisCode);
        formatter.Sync(TEXT("Severity"), nSeverity);
        formatter.Sync(TEXT("Desc"), strDesc);
        formatter.Sync(TEXT("DescInternational"), mapDescInternational);
    }
};

struct ST_DETECTION : public core::IFormatterObject
{
    std::vector<ST_EVENT> Event;

    void OnSync(core::IFormatter& formatter)
    {
        formatter.Sync(TEXT("Event"), Event);
    }
};

struct ST_ANALYSIS_REPORT : public core::IFormatterObject
{
    ST_SUMMARY Summary;
    std::map<std::tstring, ST_FILETARGET_ITEM> FileTarget;
    ST_DETECTION Detection;

    void OnSync(core::IFormatter& formatter)
    {
        formatter.Sync(TEXT("Summary"), Summary);
        formatter.Sync(TEXT("FileTarget"), FileTarget);
        formatter.Sync(TEXT("Detection"), Detection);
    }
};
