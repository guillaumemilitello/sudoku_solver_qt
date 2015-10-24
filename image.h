#ifndef IMAGE_H
#define IMAGE_H

#include <opencv2/opencv.hpp>
#include <QImage>

#include "ocr.h"

using namespace std;
using namespace cv;

// ratio case/number in a case
const int case_size = matching_size * 1.6;

// increase case/number size while cropping a case/number
const int crop_size = matching_size * 0.3;

QImage Mat2QImage(cv::Mat const& src);
cv::Mat QImage2Mat(QImage const& src);
Mat processInput(Mat img);
void transformSudoku(Mat img_src, Mat& img_tr, Rect& big_rect);
void segmentSudoku(Mat img, Mat& img_num, int x, int y);

#endif // IMAGE_H

