#include "ClassCroppingStats.hpp"
#include <iostream>

ClassCroppingStats::ClassCroppingStats()
{
    _totalImages = 0;
    _totalFrontalFacesFound = 0;
    _totalProfileFacesFound = 0;
    _totalFlippedProfileFacesFound = 0;
    _totalProfileLowScaleFactorFacesFound = 0;
    _totalFlippedProfileLowScaleFactorFacesFound = 0;
    _totalFacesNotFound = 0;
}

void ClassCroppingStats::PrintStats()
{
    std::cout << "TotalImages: " << _totalImages << std::endl;
    std::cout << "Total Frontal Faces Found: " << _totalFrontalFacesFound << std::endl;
    std::cout << "Total Profile Faces Found: " << _totalProfileFacesFound << std::endl;
    std::cout << "Total Flipped Profile Faces Found: " << _totalFlippedProfileFacesFound << std::endl;
    std::cout << "Total Profile (Low Scale Factor) Faces Found: " << _totalProfileLowScaleFactorFacesFound << std::endl;
    std::cout << "Total Flipped Profile (Low Scale Factor) Faces Found: " << _totalFlippedProfileLowScaleFactorFacesFound << std::endl;
    std::cout << "Total Images With No Faces Found: " << _totalFacesNotFound << std::endl;
}
