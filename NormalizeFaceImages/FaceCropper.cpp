#include "FaceCropper.hpp"

FaceCropper::FaceCropper()
{
    _frontalFaceDetector = cv::CascadeClassifier();
    if (!_frontalFaceDetector.load("/home/elli/Documents/Dropbox/The Cooper Union/MastersThesis/Code/FacialRecognition/include/haarcascade_frontalface_alt.xml"))
    {
	std::cerr << "Error loading frontalFaceDetector cascade trainer!!" << std::endl;
	exit(-1);
    }
    _profileFaceDetector = cv::CascadeClassifier();
    if (!_profileFaceDetector.load("/home/elli/Documents/Dropbox/The Cooper Union/MastersThesis/Code/FacialRecognition/include/haarcascade_profileface.xml"))
    {
	std::cerr << "Error loading profileFaceDetector cascade trainer!!" << std::endl;
	exit(-1);
    }
    
    classStats = ClassCroppingStats();
}

cv::Rect FaceCropper::CropFaceImage(cv::Mat image)
{   
    std::vector<cv::Rect> returnedFaces;
    _frontalFaceDetector.detectMultiScale(image, returnedFaces, 1.1, 3, 0, minFaceSize, maxFaceSize);
    _totalImages++;
    _totalFrontalFacesFound++;
    
    if (returnedFaces.size() == 0)
    {
	_totalFrontalFacesFound--; _totalProfileFacesFound++;
	_profileFaceDetector.detectMultiScale(image, returnedFaces, 1.0075, 4, 0, minFaceSize, maxFaceSize);
	std::cout << "Using profileFaceDetector." << std::endl;
    }
    if (returnedFaces.size() == 0)
    {
	_totalProfileFacesFound--; _totalFlippedProfileFacesFound++;
	cv::Mat flippedImage;
	cv::flip(image, flippedImage, 1);
	_profileFaceDetector.detectMultiScale(flippedImage, returnedFaces, 1.0075, 4, 0, minFaceSize, maxFaceSize);
	std::cout << "Using profileFaceDetector." << std::endl;
	//need to flip the rectangles as well
	for (int i = 0; i < returnedFaces.size(); i++)
	{
	    returnedFaces[i].x = flippedImage.size().width - returnedFaces[i].x - returnedFaces[i].width;
	}
    }
    
    if (returnedFaces.size() == 0)
    {
	_totalFlippedProfileFacesFound--; _totalProfileLowScaleFactorFacesFound++;
	_profileFaceDetector.detectMultiScale(image, returnedFaces, 1.002, 3, 0, minFaceSize, maxFaceSize);
	std::cout << "Using profileFaceDetector2." << std::endl;
    }
    if (returnedFaces.size() == 0)
    {
	_totalProfileLowScaleFactorFacesFound--; _totalFlippedProfileLowScaleFactorFacesFound++;
	cv::Mat flippedImage;
	cv::flip(image, flippedImage, 1);
	_profileFaceDetector.detectMultiScale(flippedImage, returnedFaces, 1.002, 3, 0, minFaceSize, maxFaceSize);
	std::cout << "Using profileFaceDetector2." << std::endl;
	//need to flip the rectangles as well
	for (int i = 0; i < returnedFaces.size(); i++)
	{
	    returnedFaces[i].x = flippedImage.size().width - returnedFaces[i].x - returnedFaces[i].width;
	}
    }
    
    if (returnedFaces.size())
    {
	if (returnedFaces.size() > 1)
	{
	    std::cout << "Found multiple (" << returnedFaces.size() << ") face matches." << std::endl;
	}
	cv::Rect faceRectangle = returnedFaces[0];
	for (int i = 1; i < returnedFaces.size(); i++)
	{
	    if (returnedFaces[i].area() > faceRectangle.area())
	    {
		faceRectangle = returnedFaces[i];
	    }
	}
	
	cv::Rect finalFaceRect = SafeBorderAdd(faceRectangle, image.size(), BorderExtension);
	return finalFaceRect;
    }
    else
    {
	_totalFlippedProfileLowScaleFactorFacesFound--; _totalFacesNotFound++;
	std::cout << "No face matches." << std::endl;
    }
    return cv::Rect(0,0,0,0);
}

cv::Rect FaceCropper::SafeBorderAdd(cv::Rect originalRect, cv::Size imageSize, int extensionAmount)
{
    int newX = originalRect.x > extensionAmount ? originalRect.x - extensionAmount : 0;
    int newY = originalRect.y > extensionAmount ? originalRect.y - extensionAmount : 0;
    int addedWidth = originalRect.x - newX;
    int addedHeight = originalRect.y - newY;
    int newWidth = newX + originalRect.size().width + addedWidth + extensionAmount < imageSize.width ?
	originalRect.size().width + addedWidth + extensionAmount : imageSize.width - newX;
    int newHeight = newY + originalRect.size().height + addedHeight + extensionAmount < imageSize.height ?
	originalRect.size().height + addedHeight + extensionAmount : imageSize.height - newY;
    
    return cv::Rect(newX, newY, newWidth, newHeight);
}


