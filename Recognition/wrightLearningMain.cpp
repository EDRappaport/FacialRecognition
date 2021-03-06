#include <iostream>
#include <fstream>
#include <list>
#include <tuple> 
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <math.h>

#include "pugixml.hpp"
#include <dlib/matrix.h>
#include <dlib/optimization.h>
#include <opencv2/opencv.hpp>

#include "../Utilities/xmlRecordingsLoader.hpp"
#include "SharedUtils.hpp"
#include "Dictionary.hpp"
#include "L1CostFunction.hpp"
#include "DataSplitter.hpp"


void MakeDictionaries(std::list<std::tuple<cv::Size, Dictionary*>*>* sizeList, std::list<std::tuple<cv::Mat, long, std::string>> trainingList)
{
    for (std::list<std::tuple<cv::Size, Dictionary*>*>::iterator it = (*sizeList).begin(); it != (*sizeList).end(); it++)
    {
	cv::Size size = std::get<0>(**it);
	std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> resized =
	    SharedUtils::ResizeAndConvertAll(trainingList, size);
	Dictionary* dPtr = std::get<1>(**it);
	dPtr = new Dictionary(resized);
	std::get<1>(**it) = dPtr;
    }
}

bool compare(const std::tuple<long, double> first, const std::tuple<long, double> second)
{
    return std::get<1>(first) < std::get<1>(second);
}

void PrintStats(std::unordered_map<double, std::unordered_map<int, int>> correctStatsMap,
		std::unordered_map<double, std::unordered_map<int, int>> wrongStatsMap,
		std::unordered_map<double, std::unordered_map<int, std::list<double>>> rankStatsMap,
		std::unordered_map<double, std::unordered_map<int, std::list<double>>> percentageStatsMap,
		std::unordered_map<double, std::unordered_map<int, std::list<double>>> percentageStatsMap2)
{
    std::cout << "CORRECT STATS:" << std::endl;
    for (std::unordered_map<double, std::unordered_map<int, int>>::iterator mapIt = correctStatsMap.begin(); mapIt != correctStatsMap.end(); mapIt++)
    {
	std::cout << "lambda: " << mapIt->first << std::endl;
	for (std::unordered_map<int, int>::iterator mapIt2 = mapIt->second.begin(); mapIt2 != mapIt->second.end(); mapIt2++)
	{
	    std::cout << "size area: " << mapIt2->first << std::endl;
	    std::cout << "Correct Count: " << mapIt2->second << std::endl;
	}
    }
    
    std::cout << "WRONG STATS:" << std::endl;
    for (std::unordered_map<double, std::unordered_map<int, int>>::iterator mapIt = wrongStatsMap.begin(); mapIt != wrongStatsMap.end(); mapIt++)
    {
	std::cout << "lambda: " << mapIt->first << std::endl;
	for (std::unordered_map<int, int>::iterator mapIt2 = mapIt->second.begin(); mapIt2 != mapIt->second.end(); mapIt2++)
	{
	    std::cout << "size area: " << mapIt2->first << std::endl;
	    std::cout << "Wrong Count: " << mapIt2->second << std::endl;
	}
    }
    
    std::cout << "RANK STATS:" << std::endl;
    for (std::unordered_map<double, std::unordered_map<int, std::list<double>>>::iterator mapIt = rankStatsMap.begin(); mapIt != rankStatsMap.end(); mapIt++)
    {
	std::cout << "lambda: " << mapIt->first << std::endl;
	for (std::unordered_map<int, std::list<double>>::iterator mapIt2 = mapIt->second.begin(); mapIt2 != mapIt->second.end(); mapIt2++)
	{
	    std::cout << "size area: " << mapIt2->first << std::endl;
	    double sum = 0.0;
	    for (std::list<double>::iterator it = mapIt2->second.begin(); it != mapIt2->second.end(); it++)
	    {
		sum += *it;
	    }
	    double len = mapIt2->second.size();
	    std::cout << "Avg Rank: " << sum/len << std::endl;
	}
    }
 
    std::cout << "PERCENTAGE STATS:" << std::endl;
    for (std::unordered_map<double, std::unordered_map<int, std::list<double>>>::iterator mapIt = percentageStatsMap.begin(); mapIt != percentageStatsMap.end(); mapIt++)
    {
	std::cout << "lambda: " << mapIt->first << std::endl;
	for (std::unordered_map<int, std::list<double>>::iterator mapIt2 = mapIt->second.begin(); mapIt2 != mapIt->second.end(); mapIt2++)
	{
	    std::cout << "size area: " << mapIt2->first << std::endl;
	    double sum = 0.0;
	    for (std::list<double>::iterator it = mapIt2->second.begin(); it != mapIt2->second.end(); it++)
	    {
		sum += *it;
	    }
	    double len = mapIt2->second.size();
	    std::cout << "Avg Percentage: " << sum/len << std::endl;
	}
    }
    
    std::cout << "PERCENTAGE STATS2:" << std::endl;
    for (std::unordered_map<double, std::unordered_map<int, std::list<double>>>::iterator mapIt = percentageStatsMap2.begin(); mapIt != percentageStatsMap2.end(); mapIt++)
    {
	std::cout << "lambda: " << mapIt->first << std::endl;
	for (std::unordered_map<int, std::list<double>>::iterator mapIt2 = mapIt->second.begin(); mapIt2 != mapIt->second.end(); mapIt2++)
	{
	    std::cout << "size area: " << mapIt2->first << std::endl;
	    double sum = 0.0;
	    for (std::list<double>::iterator it = mapIt2->second.begin(); it != mapIt2->second.end(); it++)
	    {
		sum += *it;
	    }
	    double len = mapIt2->second.size();
	    std::cout << "Avg Percentage2: " << sum/len << std::endl;
	}
    }
}

int main(int argc, char** argv)
{
    const double trainingRatio = 0.8;  
    
    std::string xmlFileName, imageBasePath, alphaOutputs, statsOutputs;
    SharedUtils::InputParser(argc, argv, &xmlFileName, &imageBasePath, &alphaOutputs, &statsOutputs);
    
    std::ofstream sparseCodesFile;
    sparseCodesFile.open(alphaOutputs);
    
    pugi::xml_document* doc = XmlRecordingsLoader::LoadXmlDoc(xmlFileName);
    std::unordered_map<long, std::list<Recording>> recordingMap = XmlRecordingsLoader::GetRecordingsMap(doc, imageBasePath);
    
    DataSplitter ds = DataSplitter(recordingMap, trainingRatio);
    std::list<std::tuple<cv::Mat, long, std::string>> trainingList = ds.GetTrainingList();
    std::list<std::tuple<cv::Mat, long, std::string>> testList = ds.GetTestingList();
    
    column_vector startingPoint(trainingList.size());
    
    Dictionary* n = nullptr;    
    std::list<std::tuple<cv::Size, Dictionary*>*> sizeList;
    std::tuple<cv::Size, Dictionary*> t1 = std::make_tuple(cv::Size(8, 8), n);
    std::tuple<cv::Size, Dictionary*> t2 = std::make_tuple(cv::Size(12, 12), n);
    std::tuple<cv::Size, Dictionary*> t3 = std::make_tuple(cv::Size(16, 16), n);
    std::tuple<cv::Size, Dictionary*> t4 = std::make_tuple(cv::Size(24, 24), n);
    std::tuple<cv::Size, Dictionary*> t5 = std::make_tuple(cv::Size(32, 32), n);
    std::tuple<cv::Size, Dictionary*> t6 = std::make_tuple(cv::Size(64, 64), n);
    sizeList.push_back(&t1);
    sizeList.push_back(&t2);
    sizeList.push_back(&t3);
    sizeList.push_back(&t4);
    sizeList.push_back(&t5);
    sizeList.push_back(&t6);
    
    MakeDictionaries(&sizeList, trainingList);

    bool printedYet = false;
    
    std::unordered_map<double, std::unordered_map<int, int>> correctStatsMap;
    std::unordered_map<double, std::unordered_map<int, int>> wrongStatsMap;
    std::unordered_map<double, std::unordered_map<int, std::list<double>>> rankStatsMap;
    std::unordered_map<double, std::unordered_map<int, std::list<double>>> percentageStatsMap;
    std::unordered_map<double, std::unordered_map<int, std::list<double>>> percentageStatsMap2;
    
    for (double lambda = .02; lambda < 0.2; lambda += .015)
    {
	lambda = .005;
	std::cout << "Lambda: " << lambda << std::endl;
	for (std::list<std::tuple<cv::Mat, long, std::string>>::iterator it = testList.begin(); it != testList.end(); it++)
	{
	    dlib::set_colm(startingPoint, 0) = 0;
	    cv::Mat currentImage = std::get<0>(*it);  

	    for (std::list<std::tuple<cv::Size, Dictionary*>*>::iterator sizeIterator = sizeList.begin(); sizeIterator != sizeList.end(); sizeIterator++)
	    {
		// setup
		cv::Size size = std::get<0>(**sizeIterator);
		dlib::matrix<uchar> currentMatrix = SharedUtils::ResizeAndConvert(currentImage, size);
		Dictionary* D = std::get<1>(**sizeIterator);
		std::cout << D << std::endl;
		dlib::matrix<double> dict = D->GetD();
		std::vector<std::tuple<long, std::string>> idMap = D->GetIdMap();
				
		L1CostFunction costFunc = L1CostFunction(dict, lambda, currentMatrix);
		//
		
		
		// find \alpha
		dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
				dlib::objective_delta_stop_strategy(1e-5, 40).be_verbose(),
				costFunc, startingPoint, -1);
		
		column_vector threshedAlpha(trainingList.size());
		for (int ii = 0; ii < startingPoint.nr(); ii++)
		{
		    threshedAlpha(ii) = startingPoint(ii) > std::sqrt(2*lambda) ? startingPoint(ii) : 0;
		    std::cout << startingPoint(ii) << ", " << threshedAlpha(ii) << std::endl;
		}
		//

		
		// print sparse code stuffs
		if (!printedYet)
		{
		    printedYet = true;
		    sparseCodesFile << "[";
		    for (int i = 0; i < idMap.size(); i++)
		    {
			sparseCodesFile <<  std::get<0>(idMap[i]) << ", ";
		    }
		    sparseCodesFile << "]" << std::endl << "[";
		    for (int i = 0; i < idMap.size(); i++)
		    {
			sparseCodesFile <<  std::get<1>(idMap[i]) << ", ";
		    }
		    sparseCodesFile << "]" << std::endl;
		}
		
		sparseCodesFile << "Looking at recordingId: " << std::get<2>(*it) << " for subjectId: "
			<< std::get<1>(*it) << std::endl;
		sparseCodesFile << "Lambda: " << lambda << "; Size: (" << size.width << ", " << size.height << ")" << std::endl;
		sparseCodesFile << "[ ";
		for (int i = 0; i < trainingList.size(); i++)
		{
		    sparseCodesFile << threshedAlpha(i) << ", ";
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
			difMap.emplace(thisId, threshedAlpha(i)*thisCol);
		    }
		    else
		    {
			difMap[thisId] =  difMap[thisId] += threshedAlpha(i)*thisCol;
		    }
		}
		
		column_vector signalFull = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(currentMatrix));
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
		long bestGuess = std::get<0>(subjectIdToDiff.front());
		std::cout << "Guessing " << bestGuess << " for " << correctId << std::endl;
		
		double sumOfDiffs = 0;
		double sumOfDiffs2 = 0;
		for (std::list<std::tuple<long, double>>::iterator subjectIdToDiffIt = subjectIdToDiff.begin(); subjectIdToDiffIt != subjectIdToDiff.end(); subjectIdToDiffIt++)
		{
		    sumOfDiffs += 1.0 / std::get<1>(*subjectIdToDiffIt);
		    sumOfDiffs2 += 1.0 / exp(std::get<1>(*subjectIdToDiffIt) * std::get<1>(*subjectIdToDiffIt));
		}
		
		int rank = subjectIdToDiff.size();
		double guessPercentage = 0;
		double guessPercentage2 = 0;
		std::list<std::tuple<long, double>>::iterator guessIt = subjectIdToDiff.begin();
		for (int counter = 0; counter < subjectIdToDiff.size(); counter++, guessIt++)
		{
		    std::tuple<long, double> thisTuple = *guessIt;		    ;
		    if (std::get<0>(thisTuple) == correctId)
		    {
			guessPercentage = (1/std::get<1>(thisTuple))/sumOfDiffs;
			guessPercentage2 = (1.0 / exp(std::get<1>(thisTuple) * std::get<1>(thisTuple))) / sumOfDiffs2;
			rank = counter;
			break;
		    }
		}
		std::cout << "Rank: " << rank << "; GuessPercentage: " << guessPercentage << "; GuessPercentage2: " << guessPercentage2 << std::endl;
		
		bool correct = bestGuess == correctId;
		
		int sizeSize = size.width * size.height;
		if (correct)
		{
		    if (correctStatsMap.find(lambda) == correctStatsMap.end())
		    {
			std::unordered_map<int, int> tempMap;
			correctStatsMap.emplace(lambda, tempMap);
		    }
		    std::unordered_map<int, int> localMap = correctStatsMap.find(lambda)->second;
		    if (localMap.find(sizeSize) == localMap.end())
		    {
			localMap.emplace(sizeSize, 0);
		    }
		    localMap.find(sizeSize)->second = localMap.find(sizeSize)->second+1;
		    correctStatsMap.find(lambda)->second = localMap;
		}
		else
		{
		    if (wrongStatsMap.find(lambda) == wrongStatsMap.end())
		    {
			std::unordered_map<int, int> tempMap;
			wrongStatsMap.emplace(lambda, tempMap);
		    }
		    std::unordered_map<int, int> localMap = wrongStatsMap.find(lambda)->second;
		    if (localMap.find(sizeSize) == localMap.end())
		    {
			localMap.emplace(sizeSize, 0);
		    }
		    localMap.find(sizeSize)->second = localMap.find(sizeSize)->second+1;
		    wrongStatsMap.find(lambda)->second = localMap;
		}
		
		
		if (rankStatsMap.find(lambda) == rankStatsMap.end())
		{
		    std::unordered_map<int, std::list<double>> tempMap;
		    rankStatsMap.emplace(lambda, tempMap);
		}
		std::unordered_map<int, std::list<double>> localMap = rankStatsMap.find(lambda)->second;
		if (localMap.find(sizeSize) == localMap.end())
		{
		    localMap.emplace(sizeSize, std::list<double>());
		}
		localMap.find(sizeSize)->second.push_back(rank);
		rankStatsMap.find(lambda)->second = localMap;
		
		if (percentageStatsMap.find(lambda) == percentageStatsMap.end())
		{
		    std::unordered_map<int, std::list<double>> tempMap;
		    percentageStatsMap.emplace(lambda, tempMap);
		}
		std::unordered_map<int, std::list<double>> localMap2 = percentageStatsMap.find(lambda)->second;
		if (localMap2.find(sizeSize) == localMap2.end())
		{
		    localMap2.emplace(sizeSize, std::list<double>());
		}
		localMap2.find(sizeSize)->second.push_back(guessPercentage);
		percentageStatsMap.find(lambda)->second = localMap2;
		
		if (percentageStatsMap2.find(lambda) == percentageStatsMap2.end())
		{
		    std::unordered_map<int, std::list<double>> tempMap;
		    percentageStatsMap2.emplace(lambda, tempMap);
		}
		std::unordered_map<int, std::list<double>> localMap3 = percentageStatsMap2.find(lambda)->second;
		if (localMap3.find(sizeSize) == localMap2.end())
		{
		    localMap3.emplace(sizeSize, std::list<double>());
		}
		localMap3.find(sizeSize)->second.push_back(guessPercentage2);
		percentageStatsMap2.find(lambda)->second = localMap3;
		
		PrintStats(correctStatsMap, wrongStatsMap, rankStatsMap, percentageStatsMap, percentageStatsMap2);
	    }
	}
    }
    
    sparseCodesFile.close();
}