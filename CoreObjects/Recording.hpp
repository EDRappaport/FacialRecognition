#ifndef _RECORDING_H
#define _RECORDING_H

#include <stdio.h>
#include <iostream>

class Recording
{
public:
    Recording(std::string recordingId, std::string subjectId, std::string basePath, std::string fileRoot, std::string filePath, bool hasFace = false);
    std::string GetFilePath();
    std::string GetRelativePath();
    std::string GetRecordingId();
    long GetSubjectId();
    bool HasFace();
    
private:
    std::string _basePath;
    std::string _relativePath;
    std::string _recordingId;
    long _subjectId;
    bool _hasFace;
    
    std::string CreateFilePath(std::string fileRoot, std::string filePath);
};

#endif