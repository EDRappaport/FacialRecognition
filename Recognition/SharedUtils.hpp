#ifndef _SHAREDUTILS_H
#define _SHAREDUTILS_H

#include <iostream>
#include <getopt.h>
#include <stdlib.h>

class SharedUtils
{
public:
    static void PrintUsage(std::string progName);
    static void InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath,
			    std::string* alphaOutputs, std::string* statsOutputs);
};

#endif