#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;
using namespace cv::ml;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Mat img_src;
    Mat img_solved;

    Rect sudoku_rect;

    int grid[9][9], grid_solved[9][9];

    void parse_contours();

private slots:

    void on_pushButton_open_pressed();
    void on_pushButton_solve_pressed();

    void on_actionLoad_OCR_triggered();
    void on_actionTrain_OCR_triggered();
    void on_pushButton_OCR_next_clicked();
    void on_pushButton_OCR_quit_pressed();
    void on_actionSave_OCR_triggered();
    void on_pushButton_OCR_train_clicked();

    void update_mainWindow();

    void train_ocr();

    void show_warning(const QString s);

private:
    Ui::MainWindow *ui;

    QImage  *imageObject;
    QGraphicsScene *scene_input;
    QGraphicsScene *scene_ocr;
    QGraphicsScene *scene_solved;

    // image
    QPixmap image;
    QPixmap image_display;
    double ratio_width;
    double ratio_height;

    // ocr data
    Mat samples, responses;
    Ptr<KNearest> knn;
    int ocr_number;

    bool ocr_training;
    bool ocr_ready;
    bool img_ready;

    // ocr training
    Mat img_p, img_contours, img_num;

    unsigned long it_contours;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Rect contour_rect;
    QGraphicsRectItem *display_rect;

    // warning box
    QWidget *warning_window;
};

#endif // MAINWINDOW_H
