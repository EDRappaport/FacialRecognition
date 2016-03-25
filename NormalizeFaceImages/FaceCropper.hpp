#ifndef _FACECROPPER_H
#define _FACECROPPER_H

#include "../CoreObjects/Recording.hpp"
#include "ClassCroppingStats.hpp"

#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

const cv::Size minFaceSize = cv::Size(200, 200);
const cv::Size maxFaceSize = cv::Size(475, 475);
const int BorderExtension = 15;

class FaceCropper : public ClassCroppingStats
{
public:
    FaceCropper();
    cv::Rect CropFaceImage(cv::Mat image);
    ClassCroppingStats classStats;
    
private:
    cv::CascadeClassifier _frontalFaceDetector;
    cv::CascadeClassifier _profileFaceDetector;
    
    cv::Rect SafeBorderAdd(cv::Rect originalRect, cv::Size imageSize, int extensionAmount);
};

#endif