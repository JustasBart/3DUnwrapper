#-------------------------------------------------
#
# Project created by QtCreator 2017-01-16T10:37:33
#
#-------------------------------------------------

QT  += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QT  += serialport

TARGET = openCV_PC_Template
TEMPLATE = app

QMAKE_CXXFLAGS_DEBUG += -O0

SOURCES += main.cpp\
    mainwindow.cpp \
    QT-OPENCV-Lib.cpp

HEADERS  += mainwindow.h \
    QT-OPENCV-Lib.h

FORMS   += mainwindow.ui

INCLUDEPATH += /usr/local/include/

#LIBS += /usr/local/lib/opencv_calib3d

#    -lopencv_calib3d \

LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui

## this path ma have to change to wherever your "libopencv_*.so" files are stored on your pc
LIBS += -L/usr/local/lib/x86_64-linux-gnu \
    -lopencv_calib3d \
    -lopencv_videoio \
    -lopencv_core \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_stitching \
    -lopencv_superres \
    -lopencv_video \
    -lopencv_imgcodecs \
    -lopencv_videostab
