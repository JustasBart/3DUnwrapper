#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QT-OPENCV-Lib.h>
#include <QTime>
#include <math.h>

#include <QSerialPort>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

#define OMNI_SERIAL         "ttyUSB1"
#define OPTI_SCAN_SERIAL    "ttyUSB0"

#define FOCUS_ENERGY        500

// #define MOTOR_SLEEP
// #define RULER_GUIDE
// #define TEST_IMAGE_STACKING

// #define UN8NUMBEROFIMAGES   30
// #define ROI_GROWTH          8

#define IMAGE_WIDTH         1920
#define IMAGE_HEIGHT        1080

#define STEP_MM_MULTIPLIER  4000

#define REPLY_ATTEMPTS      3

// Guides sizes: 1 - 8
#define GUIDES_SIZE 5

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

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
    void on_stackButton_pressed();
    void on_laplacianButton_pressed();
    void on_guidesButon_pressed();
    void on_lowerBoundry_pressed();
    void on_upperBoundry_pressed();
    void on_minFocus_pressed();
    void on_maxFocus_pressed();
    void on_layersSpinBox_valueChanged(int arg1);
    void on_strengthSpinBox_valueChanged(int arg1);
    void on_roiGrowthSpinBox_valueChanged(int arg1);
    void on_areaSpinBox_valueChanged(int arg1);

public slots:

protected:

private:

    enum class GantryPosition
    {
        Left,
        Right,
        Up,
        Down
    };

    bool laplacianMode;
    bool guidesToggle;

    bool cancelStack;

    Ui::MainWindow *ui;
    Mat LiveImage;
    Mat PreviewImage;
    QTimer VideoTimer;

    QPixmap pixLiveImage;

    Mat LapGrey;
    Mat LapMat;

    double focus = 0;

    double bestFocus = 0;
    int bestImage = 0;

    cv::Rect roi;

    int focusHeight;
    int upperFocus, lowerFocus;
    int focusBy;

    int focus_area_size;

    QSerialPort serialOptiScan;
    QSerialPort serialOmni;

    int UN8NUMBEROFIMAGES;
    int ROI_GROWTH;
    int ENERGY_MULTIPLIER;

    bool stackAllow[2];

    /////////////////////////////////////////////////

    void handleSerialCommunications();
    void enableButtons(bool state);
    void StackImagesFocusFast(Mat threeDimentional);

    void pN(double num);
    void pM(QString mes);
    void pS(cv::Size s);

    void testImage(Mat img);
    void qSleep(int ms);
    void drawGuide(Mat mat, int guideHeight, int guideThickness, int guideSpacing);

    int findFocusLevel(void);
    double findMoveZoomRatio(void);

    void checkStackStatus();

    void processHeight(Mat heigthMat[], Mat finalMat);
    void expandROI(int i, int j, cv::Rect *region, int position);
    void moveGantry(GantryPosition instruction, double millimeter);
    void zoomOutHeight(void);

    void increaseROI(int width, int height, cv::Rect *region, int position);
};

#endif // MAINWINDOW_H
