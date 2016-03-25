#ifndef _CLASSCROPPINGSTATS_H
#define _CLASSCROPPINGSTATS_H

class ClassCroppingStats
{
public:
    ClassCroppingStats();
    void PrintStats();
protected:
    int _totalImages;
    int _totalFrontalFacesFound;
    int _totalProfileFacesFound;
    int _totalFlippedProfileFacesFound;
    int _totalProfileLowScaleFactorFacesFound;
    int _totalFlippedProfileLowScaleFactorFacesFound;
    int _totalFacesNotFound;
};

#endif