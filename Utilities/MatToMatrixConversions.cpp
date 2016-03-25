#include "MatToMatrixConversions.hpp"

dlib::matrix<unsigned char> MatToMatrixConversions::MatToMatrix(cv::Mat mat)
{
    if (mat.depth() != CV_8U || mat.channels() != 1)
    {
	std::cerr << "Expecting grayscale unsigned char image!" << std::endl;
	throw;
    }
    
    dlib::matrix<unsigned char> result(mat.rows, mat.cols);
    
    unsigned char* p;
    for(int i = 0; i < mat.rows; i++)
    {
	p = mat.ptr<unsigned char>(i);
	for (int j = 0; j < mat.cols; j++)
	{
	    result(i,j) = p[j];
	}
    }
    
    return result;
}
