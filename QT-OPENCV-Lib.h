#ifndef QTOPENCVLIB_H
#define QTOPENCVLIB_H

#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "QTimer"
#include <QTime>
#include <QImage>
#include <QPixmap>
#include <QLabel>

using namespace cv;
using namespace std;

QImage MatToQImage(const Mat&);
QPixmap MatToQPixmap(const Mat&);
void RotateVideo(Mat&, Mat&, double);

// For Signals and Slots
typedef cv::Mat SignalMat;

#endif // QTOPENCVLIB_H
