#include "xmlRecordingsLoader.hpp"

std::unordered_map< long int, std::list< Recording > > XmlRecordingsLoader::GetRecordingsMap(pugi::xml_document* xmlDoc, std::string imageBasePath)
{
    std::unordered_map< long int, std::list< Recording > > map;
      
    pugi::xml_node recordingsNode = (*xmlDoc).child("Recordings");
    pugi::xml_object_range<pugi::xml_named_node_iterator> recordingsIterator = (*xmlDoc).child("Recordings")
	.children("Recording");
    int totalImages = 0;
    for (pugi::xml_named_node_iterator it = recordingsIterator.begin(); it != recordingsIterator.end(); it++)
    {
	std::string recordingId = (*it).attribute("id").value();
	std::string fileRoot = (*it).child("URL").attribute("root").value();
	std::string filePathWithExtension = (*it).child("URL").attribute("relative").value();
	std::string filePath = filePathWithExtension.substr(0, filePathWithExtension.length()-4);
	std::string subject = (*it).child("Subject").attribute("id").value();
	
	pugi::xml_node facePos = (*it).child("Subject").child("Application").child("Face").child("FacePosition");
	
	Recording newRecording = Recording(recordingId, subject, imageBasePath, fileRoot, filePath, facePos != nullptr);
	
	if (map.find(newRecording.GetSubjectId()) == map.end())
	{
	    map.emplace(newRecording.GetSubjectId(), std::list<Recording>());
	}
	
	map.find(newRecording.GetSubjectId())->second.push_back(newRecording);
	totalImages++;
    }
    std::cout << "Total Images in DataSet: " << totalImages << std::endl;
    return map;
}


std::list< Recording > XmlRecordingsLoader::GetAllRecordings(pugi::xml_document* xmlDoc, std::string imageBasePath)
{
    std::list<Recording> recordingList;  
    
    int totalImages = 0;
    int totalSubjects = 0;
    std::unordered_map< long int, std::list< Recording > > map = GetRecordingsMap(xmlDoc, imageBasePath);
    for ( std::unordered_map< long int, std::list< Recording > >::const_iterator it = map.begin(); it != map.end(); it++ )
    {
	totalSubjects++;
	for (std::list< Recording >::const_iterator listIt = it->second.begin(); listIt != it->second.end(); listIt++)
	{
	    recordingList.push_back(*listIt);
	    totalImages++;
	}
    }

    std::cout << "Total Images in DataSet: " << totalImages << std::endl;
    std::cout << "Total Subjects in DataSet: " << totalSubjects << std::endl;
    return recordingList;
}


pugi::xml_document* XmlRecordingsLoader::LoadXmlDoc(std::string xmlFileName)
{
    pugi::xml_document* doc = new pugi::xml_document;
    if (!doc->load_file(xmlFileName.c_str()))
    {
	std::cerr << "Failed to load XML doc \'" << xmlFileName << "\'" << std::endl;
	exit(-1);
    }
    return doc;
}
