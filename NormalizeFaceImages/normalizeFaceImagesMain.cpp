#include <iostream>
#include <getopt.h>
#include <list>
#include <sstream>
#include <libgen.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <opencv2/opencv.hpp>

#include "pugixml.hpp"
#include "../Utilities/xmlRecordingsLoader.hpp"
#include "../CoreObjects/Recording.hpp"
#include "FaceCropper.hpp"

void PrintUsage()
{
    std::cerr << "Usage: ./normalizefaceimages --xmlFileName <Ground Truths XML doc> --imageBasePath <Base Path for images> --processedImageBasePath <Base Path for processed images> --outputXmlDoc <outputFile>"
	<< std::endl;
}

void InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath,
		 std::string* processedImageBasePath, std::string* outputXmlDoc)
{
    (*xmlFileName) = "";
    (*imageBasePath) = "";
    (*processedImageBasePath) = "";
    (*outputXmlDoc) = "";
    
    static struct option long_options[] = 
    {
	{"xmlFileName", required_argument, 0, 'x'},
	{"imageBasePath", required_argument, 0, 'i'},
	{"processedImageBasePath", required_argument, 0, 'p'},
	{"outputXmlDoc", required_argument, 0, 'o'},
	{"help", no_argument, 0, 'h'},
    };
    
    int c;
    while(1)
    {
	int option_index = 0;
	c = getopt_long (argc, argv, "x:i:p:o:h", long_options, &option_index);
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
	    case 'p':
		(*processedImageBasePath) = optarg;
		break;
	    case 'o':
		(*outputXmlDoc) = optarg;
		break;
	    case 'h':
		PrintUsage();
		exit(-1);
	}
    }
    if (*xmlFileName == "" || *imageBasePath == "" || *processedImageBasePath == "" || *outputXmlDoc == "")
    {
	std::cerr << "Missing required arguments."  << std::endl;
	PrintUsage();
	exit(-1);
    }
}


// TODO: make work for Windows as well
void EnsureDirectoriesExist(std::string fullPath)
{
    char* directoryName = dirname(&fullPath[0]);
    std::vector<std::string> directoriesToCreate;
    
    struct stat myStat;
    while (!((stat(directoryName, &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR)))
    {
	directoriesToCreate.push_back(directoryName);
	directoryName = dirname(directoryName);
    }
    for (int i = directoriesToCreate.size()-1; i >= 0; i--)
    {
	int success = mkdir(&(directoriesToCreate[i][0]), 0777);
	if (success < 0)
	{
	    std::cerr << "Error creating directory: " << directoriesToCreate[i] << std::endl;
	    std::cerr << strerror(errno);
	}
    }
}


void AddFaceRectagleToXmlDoc(pugi::xml_document* doc, cv::Rect faceRectangle, std::string recordingId)
{
    pugi::xml_node faceNode = doc->child("Recordings").find_child_by_attribute("Recording", "id", recordingId.c_str())
	.child("Subject").child("Application").child("Face");
    pugi::xml_node facePositionNode =  faceNode.append_child("FacePosition");
    facePositionNode.append_attribute("x") = faceRectangle.x;
    facePositionNode.append_attribute("y") = faceRectangle.y;
    facePositionNode.append_attribute("width") = faceRectangle.width;
    facePositionNode.append_attribute("height") = faceRectangle.height;
}

int main(int argc, char** argv)
{    
    std::string xmlFileName, imageBasePath, processedImageBasePath, outputXmlDoc;
    InputParser(argc, argv, &xmlFileName, &imageBasePath, &processedImageBasePath, &outputXmlDoc);
    std::cout << xmlFileName << std::endl << imageBasePath << std::endl;

    pugi::xml_document* doc = XmlRecordingsLoader::LoadXmlDoc(xmlFileName);
    std::list<Recording> recordingList = XmlRecordingsLoader::GetAllRecordings(doc, imageBasePath);

    
    FaceCropper faceCropper = FaceCropper();
    for (std::list<Recording>::iterator it = recordingList.begin(); it != recordingList.end(); it++)
    {
	std::cout << "Looking at: " <<  it->GetRelativePath() << std::endl;
	cv::Mat currentImage = cv::imread(it->GetFilePath(), cv::IMREAD_UNCHANGED);
	cv::Rect faceRectangle = faceCropper.CropFaceImage(currentImage);	
		    
	std::string outputPath = processedImageBasePath + it->GetRelativePath();
	EnsureDirectoriesExist(outputPath);
	
	cv::Mat roi = currentImage(faceRectangle);
	cv::imwrite(outputPath, roi);
	AddFaceRectagleToXmlDoc(doc, faceRectangle, it->GetRecordingId());	
	
	std::cout << std::endl;
    }
    doc->save_file(outputXmlDoc.c_str());
    faceCropper.PrintStats();
        
    return 0;
}
