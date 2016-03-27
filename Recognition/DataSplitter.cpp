#include "DataSplitter.hpp"

#include <iostream>

DataSplitter::DataSplitter(std::unordered_map<long, std::list<Recording>> idToSamplesMap, double trainingRatio)
{
    srand (13);
    int numSubjectsUsed = 0;
    
    for (std::unordered_map<long, std::list<Recording>>::iterator it = idToSamplesMap.begin(); it != idToSamplesMap.end(); it++)
    {
	// only want ppl with 12+ face images
	if (GetFaceCount(it->second) < 12)
	{
	    continue;
	}
	
	for (std::list<Recording>::iterator listIt = it->second.begin(); listIt != it->second.end(); listIt++)
	{
	    if (!listIt->HasFace())
	    {
		continue;
	    }
	    
	    cv::Mat currentImage = cv::imread(listIt->GetFilePath(), cv::IMREAD_GRAYSCALE);
	    
	    cv::Mat equalizedCurrentImage;
	    cv::equalizeHist(currentImage, equalizedCurrentImage);
	    
	    if (rand()%10 < trainingRatio*10)
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TRAINING." << std::endl;		
		_trainingList.push_back(std::make_tuple(equalizedCurrentImage, listIt->GetSubjectId(), listIt->GetRecordingId()));
	    }
	    else
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TESTING." << std::endl;
		_testList.push_back(std::make_tuple(equalizedCurrentImage, listIt->GetSubjectId(), listIt->GetRecordingId()));	    
	    }
	}	
	
	if (numSubjectsUsed++ > 40)
	{
	    break;
	}
    }
    
    std::cout << "Number of Subjects Used: " << numSubjectsUsed << std::endl;
    std::cout << "Training size: " << _trainingList.size() << std::endl;
    std::cout << "Testing size: " << _testList.size() << std::endl;
    
}

int DataSplitter::GetFaceCount(std::list< Recording > recordingList)
{
    int faceImageCount = 0;
    for (std::list<Recording>::iterator listIt = recordingList.begin(); listIt != recordingList.end(); listIt++)
    {
	if (listIt->HasFace())
	{
	    faceImageCount++;
	}
    }
    return faceImageCount;
}

std::list< std::tuple< cv::Mat, long int, std::string > > DataSplitter::GetTestingList()
{
    return _testList;
}

std::list< std::tuple< cv::Mat, long int, std::string > > DataSplitter::GetTrainingList()
{
    return _trainingList;
}

