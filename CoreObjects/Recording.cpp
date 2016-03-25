#include "Recording.hpp"

#include <string.h>
#include <iostream> 

Recording::Recording(std::string recordingId, std::string subjectId, std::string basePath, std::string fileRoot, std::string filePath, bool hasFace)
{
    _recordingId = recordingId;
    _subjectId = std::stoi(subjectId.substr(4,5));
    _basePath = basePath;
    _relativePath = CreateFilePath(fileRoot, filePath);
    _hasFace = hasFace;
}

std::string Recording::CreateFilePath(std::string fileRoot, std::string filePath)
{
    std::string fullFilePath = "";
    
    if (fileRoot.compare("Disc1") == 0)
    {
	fullFilePath += "/dvd1/";
    }
    else if (fileRoot.compare("Disc2") == 0)
    {
	fullFilePath += "/dvd2/";
    }
    else
    {
	std::cerr << fileRoot << " is not either \'Disc1\' or \'Disc2\'" << std::endl;
    }
    fullFilePath += filePath;
    return fullFilePath;
}

std::string Recording::GetFilePath()
{
    return _basePath + _relativePath;
}

std::string Recording::GetRelativePath()
{
    return _relativePath;
}

std::string Recording::GetRecordingId()
{
    return _recordingId;
}

long int Recording::GetSubjectId()
{
    return _subjectId;
}

bool Recording::HasFace()
{
    return _hasFace;
}
