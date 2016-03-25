#ifndef _XMLRECORDINGSLOADER_H
#define _XMLRECORDINGSLOADER_H

#include <list>
#include <unordered_map>

#include "pugixml.hpp"
#include "../CoreObjects/Recording.hpp"

class XmlRecordingsLoader
{
public:
    static pugi::xml_document* LoadXmlDoc(std::string xmlFileName);
    static std::unordered_map<long, std::list<Recording>> GetRecordingsMap(pugi::xml_document* xmlDoc, std::string imageBasePath);
    static std::list<Recording> GetAllRecordings(pugi::xml_document* xmlDoc, std::string imageBasePath);
};


#endif
