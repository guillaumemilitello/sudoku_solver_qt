#ifndef OCR_H
#define OCR_H

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace cv::ml;

// minimum height in pix for input of train data
const int train_height_min = 9;

// matching OCR pixel size (square)
const int matching_size = 35;

// minimum matching distance to solve a number
const int minimum_matching_dist = 10000000;

int getNumberOCR(Mat img, Ptr<KNearest> knn);

#endif // OCR_H

