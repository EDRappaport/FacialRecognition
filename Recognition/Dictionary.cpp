#include "Dictionary.hpp"

Dictionary::Dictionary(std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>> trainingSet)
{
    std::cout << "Making Dictionary" << std:: endl;
    
    int trainingSize = trainingSet.size();
    long signalLength = std::get<0>(trainingSet.front()).size();
    _D.set_size(signalLength, trainingSize);
    _map = std::vector<std::tuple<long, std::string>>(trainingSize);
    
    int i =  0;
    for (std::list<std::tuple<dlib::matrix<unsigned char>, long, std::string>>::iterator it = trainingSet.begin(); it != trainingSet.end(); it++, i++)
    {
	dlib::matrix<unsigned char> mat = std::get<0>(*it);
	dlib::matrix<double> col = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(mat));
	dlib::set_colm(_D, i) = MakeUnitL2Norm(col);
	
	long subjectId = std::get<1>(*it);
	std::string recordingId = std::get<2>(*it);
	_map[i] = std::make_tuple(subjectId, recordingId);
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

std::vector<std::tuple<long, std::string>> Dictionary::GetIdMap()
{
    return _map;
}


