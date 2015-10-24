#include "mainwindow.h"
#include <QApplication>

#include <QPushButton>

/* standalone */

#include <opencv2/opencv.hpp>

#include "ocr.h"
#include "sudoku.h"
#include "image.h"

using namespace std;
using namespace cv;
using namespace cv::ml;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

