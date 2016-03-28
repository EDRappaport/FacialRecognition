#include<iostream>
#include <fstream>
#include<vector>
#include<stdlib.h>
#include<unordered_map>

#include "pugixml.hpp"
#include <dlib/matrix.h>
#include <dlib/optimization.h>
#include <opencv2/opencv.hpp>

#include "../Utilities/xmlRecordingsLoader.hpp"
#include "L1CostFunction.hpp"
#include "DataSplitter.hpp"
#include "SharedUtils.hpp"

typedef dlib::matrix<double,0,1> column_vector;

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
    
    std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> trainingData =
	SharedUtils::ResizeAndConvertAll(trainingList, cv::Size(12, 12));
    std::vector<std::tuple<dlib::matrix<unsigned char>, long, std::string>> trainingVector =
	std::vector<std::tuple<dlib::matrix<unsigned char>, long, std::string>>(trainingData.size());
	int ii =0;
    for (std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>>::iterator trainingIt = trainingData.begin(); trainingIt != trainingData.end(); trainingIt++, ii++)
    {
	trainingVector[ii] = *trainingIt;
    }
	
    dlib::matrix<double, 144, 256> dictionary;
    dlib::set_all_elements(dictionary, 0.0);
    dlib::set_subm(dictionary, dlib::range(0, 143), dlib::range(0, 143)) = dlib::identity_matrix<double>(144);
    
    std::unordered_map<std::string, column_vector> recordingIdToAlpha;
    
    double lambda = 0.1;
    for (int ii = 0; ii < 500; ii++)
    {
	int randIndex = rand() % trainingVector.size();
	std::tuple<dlib::matrix<unsigned char>, long, std::string> thisTuple = trainingVector[randIndex];
	
	dlib::matrix<unsigned char> currentMatrix = std::get<0>(thisTuple);
	std::string recordingId = std::get<2>(thisTuple);
	
	L1CostFunction costFunc = L1CostFunction(dictionary, lambda, currentMatrix);
	
	// find \alpha
	dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
						     dlib::objective_delta_stop_strategy(1e-5, 40).be_verbose(),
						     costFunc, startingPoint, -1);
    }
}