#include "Dictionary.hpp"

Dictionary::Dictionary(std::list<std::tuple<dlib::matrix<unsigned char>, long>> trainingSet)
{
    std::cout << "Making Dictionary" << std:: endl;
    
    int trainingSize = trainingSet.size();
    long signalLength = std::get<0>(trainingSet.front()).size();
    _D.set_size(signalLength, trainingSize);
    _map = std::vector<long>(trainingSize);
    
    //_map.reserve(trainingSize);
    int i =  0;
    for (std::list<std::tuple<dlib::matrix<unsigned char>, long>>::iterator it = trainingSet.begin(); it != trainingSet.end(); it++, i++)
    {
	dlib::matrix<unsigned char> mat = std::get<0>(*it);
	dlib::matrix<double> col = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(mat));
	dlib::set_colm(_D, i) = MakeUnitL2Norm(col);
	
	_map[i] = std::get<1>(*it);
    }
    
    std::cout << "Finished Making Dictionary" << std:: endl;
}

column_vector Dictionary::MakeUnitL2Norm(column_vector col)
{
    double l2Norm = std::sqrt(dlib::sum(dlib::pointwise_multiply(col, col)));
    return col/l2Norm;    
}


dlib::matrix<double> Dictionary::GetD()
{
    return _D;
}

std::vector<long> Dictionary::GetIdMap()
{
    return _map;
}


