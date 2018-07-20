#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QT-OPENCV-Lib.h>
#include <QSerialPort>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#define MOTOR_SLEEP
// #define RULER_GUIDE

#define OFFSET_COLS     0
#define OFFSET_ROWS     0

#define OFFSET_X        5
#define OFFSET_Y        5
#define OFFSET_RIGHT    5
#define OFFSET_BOTTOM   5

#define FOCUS_AREA_SIZE 4
#define UN8NUMBEROFIMAGES 10

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

// Guides sizes: 1 - 8
#define GUIDES_SIZE 2

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void slotCompute();
    void on_downButton_pressed();
    void on_upButton_pressed();
    void on_rightButton_pressed();
    void on_leftButton_pressed();
    void on_homeButton_pressed();
    void on_connectButton_pressed();
    void on_zeroButton_pressed();
    void on_gantryUpButton_pressed();
    void on_gantryDownButton_pressed();
    void on_testButton_pressed();
    void on_laplacianButton_pressed();

    void on_guidesButon_pressed();

public slots:

protected:

private:
    bool laplacianMode;
    bool guidesToggle;

    Ui::MainWindow *ui;
    Mat LiveImage;
    Mat PreviewImage;
    QTimer VideoTimer;

    QPixmap pixLiveImage;

    Mat LapGrey;
    Mat LapMat;

    Mat focusArea;
    double focus = 0;
    double bestFocus = 0;
    int bestImage = 0;
    cv::Rect roi;

    int windowGap = 5;

    QSerialPort serialOptiScan;
    QSerialPort serialZAxis;

    Mat SavedImage[UN8NUMBEROFIMAGES];
    Mat SavedScaledImage[UN8NUMBEROFIMAGES];
    Mat GreyImage[UN8NUMBEROFIMAGES];
    Mat LaplacianImage[UN8NUMBEROFIMAGES]; 
    Mat LaplacianScaledImage[UN8NUMBEROFIMAGES];

    /////////////////////////////////////////////////

    void handleSerialCommunications();
    void enableButtons(bool state);
    void StackImagesFocusFast(void);

    void pN(int num);
    void pM(QString mes);
    void pS(cv::Size s);

    void testImage(Mat img);
    void qSleep(int ms);
    void drawGuide(Mat mat, int guideHeight, int guideThickness, int guideSpacing);
};

#endif // MAINWINDOW_H
