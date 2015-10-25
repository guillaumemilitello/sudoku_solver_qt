#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "image.h"
#include "ocr.h"
#include "ocrdata.h"
#include "sudoku.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // default size
    this->setFixedSize(297, 40);
    this->setWindowTitle("Sudoku Solver");

    // warning box
    warning_window = new QWidget();

    // different scenes
    scene_input  = new QGraphicsScene(this);
    scene_ocr    = new QGraphicsScene(this);
    scene_solved = new QGraphicsScene(this);

    // main status
    ocr_training = false;
    ocr_ready = false;
    img_ready = false;

    // ocr
    knn = KNearest::create();

    // get default OCR data from ocrdata.h
    samples = Mat(samples_rows, samples_cols, CV_32FC1, samples_data);
    responses = Mat(responses_rows, responses_cols, CV_32FC1, responses_data);

    // train the default data
    train_ocr();

    // ocr training
    ocr_number = 0;
    it_contours = 0;
}

MainWindow::~MainWindow()
{
    delete warning_window;
    delete ui;
}

void MainWindow::on_pushButton_open_pressed()
{
    QString imagePath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Images (*.jpg *.jpeg *.png)" ));

    // image selectionned
    if(!imagePath.isEmpty())
    {
        imageObject = new QImage();
        imageObject->load(imagePath);

        // open input image
        image = QPixmap::fromImage(*imageObject);

        // convert to input image to cv::Mat
        img_src = QImage2Mat(*imageObject);

        // keep the ratio ans fit to maximum in case
        if(image.width() > image.height())
        {
            image_display = image.scaledToHeight(400, Qt::SmoothTransformation);

            // fit to the maximum size
            if(image_display.width() > 884)
            {
                image_display = image.scaledToWidth(884, Qt::SmoothTransformation);
            }
        }
        else
        {
            image_display = image.scaledToWidth(400, Qt::SmoothTransformation);

            // fit to the maximum size
            if(image_display.width() > 811)
            {
                image_display = image.scaledToHeight(811, Qt::SmoothTransformation);
            }
        }

        // get ration for number display
        ratio_width = (double) image_display.width() / image.width();
        ratio_height = (double) image_display.height() / image.height();

        // update the new scenes with the input image
        scene_input->clear();
        scene_ocr->clear();
        scene_solved->clear();
        scene_input->setSceneRect(0, 0, image_display.width(), image_display.height());
        scene_ocr->setSceneRect(0, 0, image_display.width(), image_display.height());
        scene_solved->setSceneRect(0, 0, image_display.width(), image_display.height());
        scene_input->addPixmap(image_display);
        scene_ocr->addPixmap(image_display);
        scene_solved->addPixmap(image_display);

        // update the graphic view
        ui->graphicsView->resize(image_display.width(), image_display.height());
        ui->graphicsView->setRenderHints(QPainter::HighQualityAntialiasing|QPainter::TextAntialiasing);
        ui->graphicsView->setScene(scene_input);

        img_ready = true;

        // resize the main window
        update_mainWindow();

        // enable train ocr button
        ui->actionTrain_OCR->setEnabled(true);

        // go back to ocr training
        if(ocr_training) on_actionTrain_OCR_triggered();
    }
}

void MainWindow::on_pushButton_solve_pressed()
{
    // transformed image, image number
    Mat img_tr, img_num;

    // get a clean transformed sudoku grid
    transformSudoku(img_src, img_tr, sudoku_rect);

    // adjust cooridinates to the display ratio and position
    int _x = sudoku_rect.x*ratio_width;
    int _y = sudoku_rect.y*ratio_height;
    int _w = sudoku_rect.width*ratio_width;
    int _h = sudoku_rect.height*ratio_height;

    // draw a blue rectangle to point data to train
    display_rect = new QGraphicsRectItem(_x, _y, _w, _h);
    display_rect->setPen(QPen(Qt::blue));
    scene_solved->addItem(display_rect);

    // switch to the solved scene
    ui->graphicsView->setScene(scene_solved);

    // pre-process the transformed input
    Mat img_p = processInput(img_tr);

    // recognize the number for each of the segment
    for (int y = 0; y < 9; ++y)
    {
        for (int x = 0; x < 9; ++x)
        {
            // get only the grid according to the coordinates
            segmentSudoku(img_p, img_num, x, y);

            // use the OCR data to recognize the number and fill the grid
            grid[x][y] = getNumberOCR(img_num, knn);
        }
    }

    // Sudoku data initialization
    Sudoku su(grid);

    // adjust the font size to the sudoku size on the image
    QFont display_font;
    display_font.setPixelSize(sudoku_rect.height * ratio_height * 0.0625);

    // check the viability of the input grid and solve the sudoku
    if (!su.checkGrid() || !sudokuSolve(su))
    {
        show_warning("Sudoku unsolvable or issue in the input grid");

        // display ocr number to check the viability of the input
        for (int y = 0; y < 9; ++y)
        {
            for (int x = 0; x < 9; ++x)
            {
                // draw only ocr number
                QGraphicsTextItem * display_number = new QGraphicsTextItem((grid[x][y])? QString::number(grid[x][y]) : "?");

                // adjust cooridinates to the display ratio and position
                int _x = sudoku_rect.x*ratio_width  + (x + 0.25) * (sudoku_rect.width*ratio_width / 9);
                int _y = sudoku_rect.y*ratio_height + (y + 0.1) * (sudoku_rect.height*ratio_height / 9);

                // draw the number
                display_number->setPos(_x, _y);
                display_number->setDefaultTextColor(Qt::red);
                display_number->setFont(display_font);
                scene_solved->addItem(display_number);
            }
        }
    }
    else
    {
        // get the solved grid
        su.getGrid(grid_solved);

        // remove original number from the solved grid for not drawing it
        for (int y = 0; y < 9; ++y)
        {
            for (int x = 0; x < 9; ++x)
            {
                // draw only solved number
                if(grid[x][y] == 0)
                {
                    QGraphicsTextItem * display_number = new QGraphicsTextItem(QString::number(grid_solved[x][y]));

                    // adjust cooridinates to the display ratio and position
                    int _x = sudoku_rect.x*ratio_width  + (x + 0.25) * (sudoku_rect.width*ratio_width / 9);
                    int _y = sudoku_rect.y*ratio_height + (y + 0.1) * (sudoku_rect.height*ratio_height / 9);

                    // draw the number
                    display_number->setPos(_x, _y);
                    display_number->setDefaultTextColor(Qt::green);
                    display_number->setFont(display_font);
                    scene_solved->addItem(display_number);
                }
            }
        }
    }
}

void MainWindow::on_actionLoad_OCR_triggered()
{
    QString ocrfilepath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("YAML (*.yml *.yaml)" ));

    if(!ocrfilepath.isEmpty())
    {
        // load data from filename
        FileStorage OCR(ocrfilepath.toStdString(), FileStorage::READ);

        // get back the data
        OCR["sample"] >> samples;
        OCR["response"] >> responses;
        OCR.release();

        train_ocr();
    }
}

void MainWindow::on_actionTrain_OCR_triggered()
{
    ocr_training = true;

    // swith to the OCR view
    ui->graphicsView->setScene(scene_ocr);

    // resize the main window and show ocr training buttons
    update_mainWindow();

    // pre-process before getting the contours
    img_p = processInput(img_src);
    img_contours = img_p.clone();

    // find all the contours
    findContours(img_contours, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    // intialize iterator
    it_contours = 0;
    contour_rect.height = 0;

    // show rectangle
    parse_contours();
}

void MainWindow::on_pushButton_OCR_next_clicked()
{
    // set the rectangle to red
    display_rect->setPen(QPen(Qt::red));

    // go to the next contour
    parse_contours();
}

void MainWindow::on_pushButton_OCR_quit_pressed()
{
    ocr_training = false;

    // re-activate solve button
    ui->pushButton_solve->setEnabled(true);

    // switch back to the input view
    ui->graphicsView->setScene(scene_input);

    // resize the main window and hide ocr training buttons
    update_mainWindow();

    // training OCR
    train_ocr();
}

void MainWindow::on_actionSave_OCR_triggered()
{
    QString ocrfilepath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("YAML (*.yml *.yaml)" ));

    if(!ocrfilepath.isEmpty())
    {
        // store data to filename
        FileStorage OCR(ocrfilepath.toStdString(), FileStorage::WRITE);
        OCR << "sample" << samples << "response" << responses;
        OCR.release();

        cout << "samples and responses saved to " << ocrfilepath.toStdString() << endl;
    }
}

void MainWindow::on_pushButton_OCR_train_clicked()
{
    // store label
    responses.push_back((float) ui->comboBox_OCR->currentIndex()+1);

    // store data
    samples.push_back(img_num.reshape(1, 1));

    // set the rectangle to green
    display_rect->setPen(QPen(Qt::green));

    // go to the next contour
    parse_contours();
}

void MainWindow::parse_contours()
{
    // reject too small rectangles in height
    while(it_contours < contours.size() && contour_rect.height <= train_height_min)
    {
        contour_rect = boundingRect(contours.at(it_contours));

        // iterate through first hierarchy level contours
        it_contours = hierarchy[it_contours][0];
    }

    // ocr terminated
    if(it_contours >= contours.size())
    {
        on_pushButton_OCR_quit_pressed();
    }

    Mat mat_tmp;

    // crop the image with the rectangle contours and resize it to the matching size
    cv::resize(img_p(contour_rect), mat_tmp, Size(matching_size, matching_size), 0, 0, INTER_LINEAR);

    // convert to float
    mat_tmp.convertTo(img_num, CV_32FC1);

    // adjust cooridinates to the display ratio and position
    int _x = contour_rect.x*ratio_width;
    int _y = contour_rect.y*ratio_height;
    int _w = contour_rect.width*ratio_width;
    int _h = contour_rect.height*ratio_height;

    // draw a blue rectangle to point data to train
    display_rect = new QGraphicsRectItem(_x, _y, _w, _h);
    display_rect->setPen(QPen(Qt::blue));
    scene_ocr->addItem(display_rect);

    // reset height to reject too small rectangles in height
    contour_rect.height = 0;
}

void MainWindow::train_ocr()
{
    // training
    if(!samples.empty() && !responses.empty())
    {
        knn->train(samples, ROW_SAMPLE, responses);
        ocr_ready = true;
    }
    else
    {
        ocr_ready = false;
        show_warning("Error while initializing the OCR");
    }
    update_mainWindow();
}

void MainWindow::update_mainWindow()
{
    // ocr training mode
    if(ocr_training && img_ready)
    {
        this->setFixedSize(image_display.width() + 52, image_display.height() + 80);

        ui->comboBox_OCR->move(ui->comboBox_OCR->x(), image_display.height() + 33);
        ui->pushButton_OCR_train->move(ui->pushButton_OCR_train->x(), image_display.height() + 31);
        ui->pushButton_OCR_next->move(ui->pushButton_OCR_next->x(), image_display.height() + 31);
        ui->pushButton_OCR_quit->move(ui->pushButton_OCR_quit->x(), image_display.height() + 31);

        ui->pushButton_solve->setEnabled(false);
    }
    else if(img_ready)
    {
        this->setFixedSize(image_display.width() + 52, image_display.height() + 55);

        ui->comboBox_OCR->move(ui->comboBox_OCR->x(), image_display.height() + 93);
        ui->pushButton_OCR_train->move(ui->pushButton_OCR_train->x(), image_display.height() + 91);
        ui->pushButton_OCR_next->move(ui->pushButton_OCR_next->x(), image_display.height() + 91);
        ui->pushButton_OCR_quit->move(ui->pushButton_OCR_quit->x(), image_display.height() + 91);

        ui->pushButton_solve->setEnabled(ocr_ready && img_ready);
    }
}

void MainWindow::show_warning(const QString s)
{
    warning_window->setMinimumSize(297, 40);
    warning_window->setWindowTitle("Warning");

    QLabel *label = new QLabel(this);
    QHBoxLayout *layout = new QHBoxLayout();
    label->setText(s);
    layout->addWidget(label);
    warning_window->setLayout(layout);
    warning_window->show();
}
