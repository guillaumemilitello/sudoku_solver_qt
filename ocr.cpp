#include "ocr.h"
#include "image.h"

int getNumberOCR(Mat img, Ptr<KNearest> knn)
{
    // clone before finding the contours
    Mat img_contours = img.clone();

    // to display the contours
    Mat img_color;
    cvtColor(img, img_color, CV_GRAY2BGR);

    // find the contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // finding all the contours
    findContours(img_contours, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    int best_number = 0, best_distance = INT32_MAX;

    // iterate through first hierarchy level contours
    for (auto i : contours)
    {
        Mat img_tmp, img_num;

        // create vectors to get the response and the distance
        vector<float> response, distance;

        // find bounding rectangle
        Rect rect = boundingRect(i);

        // only process rectangle with height correspond to the size of a number
        if(rect.height > matching_size * 0.8 && rect.height <= matching_size * 1.5)
        {
            // resize the image at the shape of the rectangle
            resize(img(boundingRect(i)), img_tmp, Size(matching_size, matching_size), 0, 0, INTER_LINEAR);

            // convert to float for training
            img_tmp.convertTo(img_num, CV_32FC1);

            // find nearest
            knn->findNearest(img_num.reshape(1, 1), 1, noArray(), response, distance);

            // show big rectangle image
            rectangle(img_color, rect.tl(), rect.br(), Scalar(0, 255, 0), 2);

            // save the best result below a distance threshold
            if(distance.at(0) < minimum_matching_dist && distance.at(0) < best_distance)
            {
                best_distance = (int) distance.at(0);
                best_number = (int) response.at(0);
            }
        }
    }
    return best_number;
}




