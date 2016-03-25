#include <iostream>
#include <fstream>
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
    std::cerr << "Usage: ./wrightLearning --xmlFileName <Ground Truths XML doc> --imageBasePath <Base Path for images> --alphaOutputs <Output filename for the sparse codes> --statsOutputs<Output filename for stats info>"
    << std::endl;
}

void InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath,
		 std::string* alphaOutputs, std::string* statsOutputs)
{
    (*xmlFileName) = "";
    (*imageBasePath) = "";
    (*alphaOutputs) = "";
    (*statsOutputs) = "";
    
    static struct option long_options[] = 
    {
	{"xmlFileName", required_argument, 0, 'x'},
	{"imageBasePath", required_argument, 0, 'i'},
	{"alphaOutputs", required_argument, 0, 'a'},
	{"statsOutputs", required_argument, 0, 's'},
    };
    
    int c;
    while(1)
    {
	int option_index = 0;
	c = getopt_long (argc, argv, "x:i:a:s:h", long_options, &option_index);
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
	    case 'a':
		(*alphaOutputs) = optarg;
		break;
	    case 's':
		(*statsOutputs) = optarg;
		break;
	    case 'h':
		PrintUsage();
		exit(-1);
	}
    }
    if (*xmlFileName == "" || *imageBasePath == "" || *alphaOutputs == "" || *statsOutputs == "")
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

dlib::matrix<unsigned char> ResizeAndConvert(cv::Mat image, cv::Size size)
{
    cv::Mat resizedCurrentImage;
    cv::resize(image, resizedCurrentImage, size);
    dlib::matrix<uchar> matrix = MatToMatrixConversions::MatToMatrix(resizedCurrentImage);
    return matrix;
}

std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> ResizeAndConvertAll(std::list<std::tuple<cv::Mat, long, std::string>> trainingList, cv::Size size)
{
    std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> ret;
    for (std::list<std::tuple<cv::Mat, long, std::string>>::iterator it = trainingList.begin(); it != trainingList.end(); it++)
    {
	cv::Mat currentImage = std::get<0>(*it);
	long id = std::get<1>(*it);
	std::string recordingId = std::get<2>(*it);

	dlib::matrix<unsigned char> matrix = ResizeAndConvert(currentImage, size);
	
	ret.push_back(std::make_tuple(matrix, id, recordingId));
    }
    return ret;
}

void MakeDictionaries(std::list<std::tuple<cv::Size, Dictionary*>> sizeList, std::list<std::tuple<cv::Mat, long, std::string>> trainingList)
{
    for (std::list<std::tuple<cv::Size, Dictionary*>>::iterator it = sizeList.begin(); it != sizeList.end(); it++)
    {
	cv::Size size = std::get<0>(*it);
	std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> resized = ResizeAndConvertAll(trainingList, size);
	Dictionary* D = new Dictionary(resized);
	
	std::get<1>(*it) = D;
    }
}

bool compare(const std::tuple<long, double> first, const std::tuple<long, double> second)
{
    return std::get<1>(first) < std::get<1>(second);
}

int main(int argc, char** argv)
{
    const double trainingRatio = 0.8;  
    
    std::string xmlFileName, imageBasePath, alphaOutputs, statsOutputs;
    InputParser(argc, argv, &xmlFileName, &imageBasePath, &alphaOutputs, &statsOutputs);
    
    std::ofstream sparseCodesFile;
    sparseCodesFile.open(alphaOutputs);
    
    pugi::xml_document* doc = XmlRecordingsLoader::LoadXmlDoc(xmlFileName);
    std::unordered_map<long, std::list<Recording>> recordingMap = XmlRecordingsLoader::GetRecordingsMap(doc, imageBasePath);
    
    srand (13);
    int numSubjectsUsed = 0;
    
    std::list<std::tuple<cv::Mat, long, std::string>> trainingList;
    std::list<std::tuple<cv::Mat, long, std::string>> testList;
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
	    	    
	    if (rand()%10 < trainingRatio*10)
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TRAINING." << std::endl;		
		trainingList.push_back(std::make_tuple(equalizedCurrentImage, listIt->GetSubjectId(), listIt->GetRecordingId()));
	    }
	    else
	    {
		std::cout << "Using " << listIt->GetRecordingId() << "(" << listIt->GetSubjectId() << ") for TESTING." << std::endl;
		testList.push_back(std::make_tuple(equalizedCurrentImage, listIt->GetSubjectId(), listIt->GetRecordingId()));	    
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
    
    
    column_vector startingPoint(trainingList.size());
    
    Dictionary* n = nullptr;    
    std::list<std::tuple<cv::Size, Dictionary*>> sizeList;
    sizeList.push_back(std::make_tuple(cv::Size(8, 8), n));
    sizeList.push_back(std::make_tuple(cv::Size(12, 12), n));
    sizeList.push_back(std::make_tuple(cv::Size(16, 16), n));
    sizeList.push_back(std::make_tuple(cv::Size(24, 24), n));
    sizeList.push_back(std::make_tuple(cv::Size(32, 32), n));
    sizeList.push_back(std::make_tuple(cv::Size(64, 64), n));
    
    MakeDictionaries(sizeList);
    
    int numCorrect = 0;
    int numWrong = 0;
    
    bool printedYet = false;
    
    std::unordered_map<double, std::unordered_map<int, int>> correctStatsMap;
    std::unordered_map<double, std::unordered_map<int, int>> wrongStatsMap;
    
    for (double lambda = .02; lambda < 0.2; lambda += .015)
    {
	std::cout << "Lambda: " << lambda << std::endl;
	for (std::list<std::tuple<cv::Mat, long, std::string>>::iterator it = testList.begin(); it != testList.end(); it++)
	{
	    dlib::set_colm(startingPoint, 0) = 0;
	    cv::Mat currentImage = std::get<0>(*it);  
	    
	    for (std::list<std::tuple<cv::Size, Dictionary*>>::iterator sizeIterator = sizeList.begin(); sizeIterator != sizeList.end(); sizeIterator++)
	    {
		// setup
		cv::Size size = std::get<0>(*sizeIterator);
		dlib::matrix<uchar> matrix = ResizeAndConvert(currentImage, size);
		
		Dictionary* D = std::get<1>(*sizeIterator);
		dlib::matrix<double> dict = D->GetD();
		std::vector<std::tuple<long, std::string>> idMap = D->GetIdMap();
				
		L1CostFunction costFunc = L1CostFunction(D, lambda, matrix);
		//
		
		
		// find \alpha
		dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
				dlib::objective_delta_stop_strategy(1e-5, 200).be_verbose(),
				costFunc, startingPoint, -1);
		//

		
		// print sparse code stuffs
		if (!printedYet)
		{
		    printedYet = true;
		    for (int i = 0; i < idMap.size(); i++)
		    {
			sparseCodesFile <<  std::get<0>(idMap[i]);
		    }
		    sparseCodesFile << std::endl;
		    for (int i = 0; i < idMap.size(); i++)
		    {
			sparseCodesFile <<  std::get<1>(idMap[i]);
		    }
		    sparseCodesFile << std::endl;
		}
		
		sparseCodesFile << "Looking at recordingId: " << std::get<2>(*it) << " for subjectId: "
			<< std::get<1>(*it) << std::endl;
		sparseCodesFile << "Lambda: " << lambda << "; Size: (" << size.width << ", " << size.height << ")" << std::endl;
		sparseCodesFile << "[ ";
		for (int i = 0; i < trainingList.size(); i++)
		{
		    std::cout << startingPoint(i) << ", ";
		}
		sparseCodesFile << "]" << std::endl;
		//
		
		
		std::unordered_map<long, column_vector> difMap;
		for (int i = 0; i < trainingList.size(); i++)
		{
		    column_vector thisCol = dlib::colm(dict, i);
		    long thisId = std::get<0>(idMap[i]);
		    if (difMap.find(thisId) == difMap.end())
		    {
			difMap.emplace(thisId, startingPoint(i)*thisCol);
		    }
		    else
		    {
			difMap[thisId] =  difMap[thisId] += startingPoint(i)*thisCol;
		    }
		}
		
		column_vector signalFull = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(std::get<0>(*it)));
		double l2NormSig = std::sqrt(dlib::sum(dlib::pointwise_multiply(signalFull, signalFull)));
		column_vector signal = signalFull/l2NormSig;
		
		long correctId = std::get<1>(*it);
		
		std::list<std::tuple<long, double>> subjectIdToDiff;
		for (std::unordered_map<long, column_vector>::iterator mapIt = difMap.begin(); mapIt != difMap.end(); mapIt++)
		{
		    double l2Norm = std::sqrt(dlib::sum(dlib::pointwise_multiply(mapIt->second, mapIt->second)));
		    column_vector guess = mapIt->second/l2Norm;
			    
		    column_vector diff = signal - guess;
		    column_vector diff2 = dlib::pointwise_multiply(diff, diff);
			    
		    double finalDiff = std::sqrt(dlib::sum(diff2)); 
		    std::cout << "Final Diff for " << mapIt->first << " : " << finalDiff << std::endl;
		    subjectIdToDiff.push_back(std::make_tuple(mapIt->first, finalDiff));
		}
		subjectIdToDiff.sort(compare);
		long bestGuess = subjectIdToDiff.front();	
		std::cout << "Guessing " << bestGuess << " for " << correctId << std::endl;
		
		double sumOfDiffs = 0;
		for (std::list<std::tuple<long, double>>::iterator subjectIdToDiffIt = subjectIdToDiff.begin(); subjectIdToDiffIt != subjectIdToDiff.end(); subjectIdToDiffIt++)
		{
		    sumOfDiffs += 1.0 / std::get<1>(*subjectIdToDiffIt);
		}
		
		int rank = subjectIdToDiff.size();
		double guessPercentage = 0;
		for (int counter = 0; counter < subjectIdToDiff.size(); counter++)
		{
		    std::tuple<long, double> thisTuple = subjectIdToDiff[counter]		    ;
		    if (std::get<0>(thisTuple) == correctId)
		    {
			guessPercentage = std::get<1>(thisTuple)/sumOfDiffs;
			rank = counter;
			break;
		    }
		}
		std::cout << "Rank: " << rank << "; GuessPercentage: " << guessPercentage << std::endl;
		
		if (bestGuess == correctId)
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
    }
    
    sparseCodesFile.close();
}