#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#include <iostream>
#include <list>
#include <tuple>
#include <vector>
#include <math.h> 

#include <dlib/matrix.h>

typedef dlib::matrix<double,0,1> column_vector;

class Dictionary
{
public:
    Dictionary(std::list<std::tuple<dlib::matrix<unsigned char>, long>> trainingSet);
    dlib::matrix<double> GetD();
    std::vector<long> GetIdMap();
private:
    dlib::matrix<double> _D;
    std::vector<long> _map;
    
    column_vector MakeUnitL2Norm(column_vector col);
};

#endif