cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
project(faceRecognition)

find_package( OpenCV REQUIRED )

include_directories("./include/pugixml-1.7/src")
add_subdirectory(include/pugixml-1.7/src)

include(include/dlib/cmake)


add_executable(normalizefaceimages NormalizeFaceImages/normalizeFaceImagesMain.cpp
    Utilities/xmlRecordingsLoader.cpp CoreObjects/Recording.cpp NormalizeFaceImages/FaceCropper.cpp
    NormalizeFaceImages/ClassCroppingStats.cpp)
target_link_libraries( normalizefaceimages ${OpenCV_LIBS})
target_link_libraries( normalizefaceimages pugixml)
install(TARGETS normalizefaceimages RUNTIME DESTINATION bin)


add_executable(wrightLearning Recognition/wrightLearningMain.cpp
    Utilities/xmlRecordingsLoader.cpp CoreObjects/Recording.cpp Utilities/MatToMatrixConversions.cpp
    Recognition/Dictionary.cpp Recognition/L1CostFunction.cpp Recognition/DataSplitter.cpp Recognition/SharedUtils.cpp)
target_link_libraries( wrightLearning ${OpenCV_LIBS})
target_link_libraries( wrightLearning pugixml)
target_link_libraries( wrightLearning dlib)
install(TARGETS wrightLearning RUNTIME DESTINATION bin)


add_executable(gradDescentDictionaryLearning Recognition/gradDescentDictionaryLearner.cpp
    Utilities/xmlRecordingsLoader.cpp CoreObjects/Recording.cpp Utilities/MatToMatrixConversions.cpp
    Recognition/DataSplitter.cpp Recognition/SharedUtils.cpp)
target_link_libraries( gradDescentDictionaryLearning ${OpenCV_LIBS})
target_link_libraries( gradDescentDictionaryLearning pugixml)
target_link_libraries( gradDescentDictionaryLearning dlib)
install(TARGETS gradDescentDictionaryLearning RUNTIME DESTINATION bin)

