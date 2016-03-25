#ifndef _MATTOMATRIXCONVERSIONS_H_
#define _MATTOMATRIXCONVERSIONS_H_

#include <opencv2/opencv.hpp>
#include <dlib/matrix.h>

class MatToMatrixConversions
{
public:
    static dlib::matrix<unsigned char> MatToMatrix(cv::Mat mat);
};

#endif