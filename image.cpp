#include "image.h"

QImage Mat2QImage(cv::Mat const& src)
{
     switch(src.type())
     {
        case CV_8UC1:
            return QImage((const uchar *) src.data, src.cols, src.rows, src.step, QImage::Format_Indexed8);
        case CV_8UC3:
            return QImage((const uchar *) src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
        case CV_8UC4:
        default:
            return QImage((const uchar *) src.data, src.cols, src.rows, src.step, QImage::Format_RGB32);
     }
}

cv::Mat QImage2Mat(QImage const& src)
{
    switch (src.format())
    {
        case QImage::Format_Indexed8:
            return cv::Mat(src.height(),src.width(),CV_8UC1,(uchar*)src.bits(),src.bytesPerLine());
        case QImage::Format_RGB888:
            return cv::Mat(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        default:
            return cv::Mat(src.height(),src.width(),CV_8UC4,(uchar*)src.bits(),src.bytesPerLine());
    }
}

Mat processInput(Mat img)
{
    Mat img_gray, img_adpth;

    // convert to gray
    if(img.channels() == 3 || img.channels() == 4)
    {
        cvtColor(img, img_gray, CV_BGR2GRAY);
    }
    else
    {
        img_gray = img.clone();
    }

    // adaptive threshold
    adaptiveThreshold(img_gray, img_adpth, 255, 1, 1, 15, 10);

    return img_adpth;
}

void transformSudoku(Mat img, Mat& img_tr, Rect& big_rect)
{
    // pre-process the input image
    Mat img_contours = processInput(img);

     // find all the contours
     vector<vector<Point> > contours;
     vector<Vec4i> hierarchy;

     findContours(img_contours, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

     // biggest rectangle
     int rect_size_max = 0;

     for(auto i: contours)
     {
         vector<Point> contours_poly;

         // approximate contours to polygons
         approxPolyDP(Mat(i), contours_poly, 4, true);

         // only process rectangle polygons
         if(contours_poly.size() == 4 && isContourConvex(contours_poly))
         {
             int rect_size = contourArea(contours_poly, false);

             // area of the polygon
             if(rect_size > rect_size_max)
             {
                 // save the biggest rectangle
                 rect_size_max = rect_size;
                 big_rect = boundingRect(Mat(contours_poly));
             }
         }
     }

     // size of the output image fitting the OCR matching size
     Size img_tr_size;
     img_tr_size.width = case_size * 9;
     img_tr_size.height = case_size * 9;

     // grid coordinates
     Point2f in_c[4], out_c[4];

     // fitting the size of the biggest rectangle
     in_c[0] = Point2f(big_rect.x,                  big_rect.y);
     in_c[1] = Point2f(big_rect.x + big_rect.width, big_rect.y);
     in_c[2] = Point2f(big_rect.x + big_rect.width, big_rect.y + big_rect.height);
     in_c[3] = Point2f(big_rect.x,                  big_rect.y + big_rect.height);

     // fitting the size of the OCR matching size
     out_c[0] = Point2f(0,                 0);
     out_c[1] = Point2f(img_tr_size.width, 0);
     out_c[2] = Point2f(img_tr_size.width, img_tr_size.height);
     out_c[3] = Point2f(0,                 img_tr_size.height);

    // perspective transform from the input image
    warpPerspective(img, img_tr, getPerspectiveTransform(in_c, out_c), img_tr_size);
}

void segmentSudoku(Mat img, Mat& img_num, int x, int y)
{
    int x_offset = max(0, x * case_size - crop_size);
    int y_offset = max(0, y * case_size - crop_size);
    int width = case_size + crop_size;
    int height =  case_size + crop_size;

    img(Rect(x_offset, y_offset, width, height)).copyTo(img_num);
}
