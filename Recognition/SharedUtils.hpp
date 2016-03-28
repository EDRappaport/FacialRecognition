#ifndef _SHAREDUTILS_H
#define _SHAREDUTILS_H

#include <iostream>
#include <list>
#include <getopt.h>
#include <stdlib.h>

#include <dlib/matrix.h>
#include <opencv2/opencv.hpp>

#include "../Utilities/MatToMatrixConversions.hpp"

class SharedUtils
{
public:
    static void PrintUsage(std::string progName);
    static void InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath,
			    std::string* alphaOutputs, std::string* statsOutputs);
    static dlib::matrix<unsigned char> ResizeAndConvert(cv::Mat image, cv::Size size);
    static std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> ResizeAndConvertAll(
	std::list<std::tuple<cv::Mat, long, std::string>> trainingList, cv::Size size);
};

#endif