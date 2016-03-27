#ifndef _DATASPLITTER_H
#define _DATASPLITTER_H

#include <stdlib.h>
#include <list>
#include <tuple> 
#include <unordered_map>

#include <opencv2/opencv.hpp>

#include "../CoreObjects/Recording.hpp"

class DataSplitter
{
public:
    DataSplitter(std::unordered_map<long, std::list<Recording>> idToSamplesMap, double trainingRatio);
    
    std::list<std::tuple<cv::Mat, long, std::string>> GetTrainingList();
    std::list<std::tuple<cv::Mat, long, std::string>> GetTestingList();
    
private:
    static int GetFaceCount(std::list<Recording> recordingList);
    
    std::list<std::tuple<cv::Mat, long, std::string>> _trainingList;
    std::list<std::tuple<cv::Mat, long, std::string>> _testList;
};

#endif