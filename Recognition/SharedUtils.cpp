#include "SharedUtils.hpp"

void SharedUtils::PrintUsage(std::string progName)
{
    std::cerr << "Usage: " << progName << " --xmlFileName <Ground Truths XML doc> --imageBasePath <Base Path for images> --alphaOutputs <Output filename for the sparse codes> --statsOutputs<Output filename for stats info>"
    << std::endl;
}

void SharedUtils::InputParser(int argc, char** argv, std::string* xmlFileName, std::string* imageBasePath,
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
		PrintUsage(argv[0]);
		exit(-1);
	}
    }
    if (*xmlFileName == "" || *imageBasePath == "" || *alphaOutputs == "" || *statsOutputs == "")
    {
	std::cerr << "Missing required arguments."  << std::endl;
	PrintUsage(argv[0]);
	exit(-1);
    }
}