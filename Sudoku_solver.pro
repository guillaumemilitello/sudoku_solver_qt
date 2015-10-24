#-------------------------------------------------
#
# Project created by QtCreator 2015-10-23T09:27:27
#
#-------------------------------------------------

QT       += core gui

ICON = sudoku.icns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sudoku_solver
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_ml \
        -lopencv_imgcodecs

SOURCES += main.cpp\
        mainwindow.cpp \
    sudoku.cpp \
    ocr.cpp \
    image.cpp

HEADERS  += mainwindow.h \
    sudoku.h \
    ocr.h \
    image.h \
    ocrdata.h

FORMS    += mainwindow.ui

RESOURCES +=

DISTFILES +=

