#include <iostream>
#include <list>
#include <tuple> 
#include <vector>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <math.h>

#include "pugixml.hpp"
#include <dlib/matrix.h>
#include <dlib/optimization.h>
#include <opencv2/opencv.hpp>

#include "../Utilities/xmlRecordingsLoader.hpp"
#include "../Utilities/MatToMatrixConversions.hpp"
#include "Dictionary.hpp"
#include "L1CostFunction.hpp"

void PrintUsage()
{
    std::cerr << "Usage: ./wrightLearning --xmlFileName <Ground Truths XML doc> --imageBasePath <Base Path for images>"
    << std::endl;
}

void InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath)
{
    (*xmlFileName) = "";
    (*imageBasePath) = "";
    
    static struct option long_options[] = 
    {
	{"xmlFileName", required_argument, 0, 'x'},
	{"imageBasePath", required_argument, 0, 'i'},
    };
    
    int c;
    while(1)
    {
	int option_index = 0;
	c = getopt_long (argc, argv, "x:i:h", long_options, &option_index);
	if (c  == -1)
	{
	    break;
	}
	switch (c)
	{
	    case 'x':
		(*xmlFileName) = optarg;
		break;
	    case 'i':
		(*imageBasePath) = optarg;
		break;
	    case 'h':
		PrintUsage();
		exit(-1);
	}
    }
    if (*xmlFileName == "" || *imageBasePath == "")
    {
	std::cerr << "Missing required arguments."  << std::endl;
	PrintUsage();
	exit(-1);
    }
}

int GetFaceCount(std::list<Recording> recordingList)
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

int main(int argc, char** argv)
{
    const double trainingRatio = 0.8;
    double lambda = .03;
    const cv::Size imageSize(24, 24);
    
    std::string xmlFileName, imageBasePath;
    InputParser(argc, argv, &xmlFileName, &imageBasePath);
    
    pugi::xml_document* doc = XmlRecordingsLoader::LoadXmlDoc(xmlFileName);
    std::unordered_map<long, std::list<Recording>> recordingMap = XmlRecordingsLoader::GetRecordingsMap(doc, imageBasePath);
    
    srand (13);
    int numSubjectsUsed = 0;
    
    std::unordered_map<long, std::list<dlib::matrix<uchar>>> training;
    std::unordered_map<long, std::list<dlib::matrix<uchar>>> testing;
    
    std::list<std::tuple<dlib::matrix<uchar>, long>> trainingList;
    std::list<std::tuple<dlib::matrix<uchar>, long>> testList;
    for (std::unordered_map<long, std::list<Recording>>::iterator it = recordingMap.begin(); it != recordingMap.end(); it++)
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
	    
	    cv::Mat resizedCurrentImage;
	    cv::resize(equalizedCurrentImage, resizedCurrentImage, imageSize);
	    dlib::matrix<uchar> matrix = MatToMatrixConversions::MatToMatrix(resizedCurrentImage);
	    
	    if (rand()%10 < trainingRatio*10)
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TRAINING." << std::endl;		
		trainingList.push_back(std::make_tuple(matrix, listIt->GetSubjectId()));
	    }
	    else
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TESTING." << std::endl;
		testList.push_back(std::make_tuple(matrix, listIt->GetSubjectId()));	    
	    }
	}	
	
	if (numSubjectsUsed++ > 40)
	{
	    break;
	}
    }
    
    std::cout << "Number of Subjects Used: " << numSubjectsUsed << std::endl;
    std::cout << "Training size: " << trainingList.size() << std::endl;
    std::cout << "Testing size: " << testList.size() << std::endl;
    
    Dictionary* D = new Dictionary(trainingList);
    dlib::matrix<double> dict = D->GetD();
    std::vector<long> idMap = D->GetIdMap();
    column_vector startingPoint(trainingList.size());
    
    int i = 0;
    int numCorrect = 0;
    int numWrong = 0;
    for (std::list<std::tuple<dlib::matrix<uchar>, long>>::iterator it = testList.begin(); it != testList.end(); it++, i++)
    {
	std::cout << "Lambda: " << lambda << std::endl;
	L1CostFunction costFunc = L1CostFunction(D, lambda, std::get<0>(*it));
		
	dlib::set_colm(startingPoint, 0) = 0;
	dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(), dlib::objective_delta_stop_strategy(1e-5, 200).be_verbose(),
						    costFunc, startingPoint, -1);

	std::cout << "Using test case for " << std::get<1>(*it) << std::endl;
	for (int i = 0; i < trainingList.size(); i++)
	{
	    std::cout << startingPoint(i) << " : " << idMap[i] << std::endl;
	}
	std::cout << "Using test case for " << std::get<1>(*it) << std::endl;
	
	std::unordered_map<long, column_vector> difMap;
	for (int i = 0; i < trainingList.size(); i++)
	{
	    column_vector thisCOl = dlib::colm(dict, i);
	    long thisId = idMap[i];
	    if (difMap.find(thisId) == difMap.end())
	    {
		difMap.emplace(thisId, startingPoint(i)*thisCOl);
	    }
	    else
	    {
		difMap[thisId] =  difMap[thisId] += startingPoint(i)*thisCOl;
	    }
	}
	
	column_vector signalFull = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(std::get<0>(*it)));
	double l2NormSig = std::sqrt(dlib::sum(dlib::pointwise_multiply(signalFull, signalFull)));
	column_vector signal = signalFull/l2NormSig;
	
	double bestDiff = 1000000000;
	long bestGuess = -1;
	for (std::unordered_map<long, column_vector>::iterator mapIt = difMap.begin(); mapIt != difMap.end(); mapIt++)
	{
	    double l2Norm = std::sqrt(dlib::sum(dlib::pointwise_multiply(mapIt->second, mapIt->second)));
	    column_vector guess = mapIt->second/l2Norm;
	 	    
	    column_vector diff = signal - guess;
	    column_vector diff2 = dlib::pointwise_multiply(diff, diff);
	 	    
	    double finalDiff = std::sqrt(dlib::sum(diff2)); 
	    std::cout << "Final Diff for " << mapIt->first << " : " << finalDiff << std::endl;
	    if (finalDiff < bestDiff)
	    {
		bestDiff = finalDiff;
		bestGuess = mapIt->first;
	    }
	}
	
	std::cout << "Guessing " << bestGuess << " for " << std::get<1>(*it) << std::endl;
	if (bestGuess == std::get<1>(*it))
	{
	    numCorrect++;
	}
	else
	{
	    numWrong++;
	}

	std::cout << "Correct: " << numCorrect << ".  Wrong: " << numWrong << "." << std::endl;
    }
}