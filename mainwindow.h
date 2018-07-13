#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QT-OPENCV-Lib.h>
#include <QSerialPort>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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

public slots:

protected:

private:
    Ui::MainWindow *ui;
    Mat LiveImage;
    Mat PreviewImage;
    QTimer VideoTimer;
    QSerialPort serial;

    /////////////////////////////////////////////////

    void handleSerialCommunications();
    void enableButtons(bool state);
};

#endif // MAINWINDOW_H
