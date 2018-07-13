#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QRect"

VideoCapture camera(0);

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&VideoTimer, SIGNAL(timeout()), this, SLOT(slotCompute()));

    VideoTimer.start(20);
}

MainWindow::~MainWindow()
{
    serial.disconnect();

    delete ui;
}

void MainWindow::slotCompute()
{
    camera >> LiveImage; // MAT

    // To find out the size of image being taken
    // qInfo("Size of: Width %d, Height %d", LiveImage.size().width, LiveImage.size().height);

    QPixmap pixLiveImage; // Pixel Mats

    pixLiveImage = MatToQPixmap( LiveImage ); // Converting MAT to Pixel Mat

    ui -> videoLabel -> setPixmap( pixLiveImage ); // Displaying Pixel Mat
}

void MainWindow::handleSerialCommunications()
{   
    serial.setPortName("ttyUSB0"); // -> Might change

    if(serial.open(QIODevice::ReadWrite))
    {
        if(!serial.setBaudRate(QSerialPort::Baud9600))
        {
            qDebug() << serial.errorString();
            return;
        }
        if(!serial.setDataBits(QSerialPort::Data8))
        {
            qDebug() << serial.errorString();
            return;
        }
        if(!serial.setParity(QSerialPort::NoParity))
        {
            qDebug() << serial.errorString();
            return;
        }
        if(!serial.setStopBits(QSerialPort::OneStop))
        {
            qDebug() << serial.errorString();
            return;
        }
        if(!serial.setFlowControl(QSerialPort::NoFlowControl))
        {
            qDebug() << serial.errorString();
            return;
        }

        enableButtons(true);
    }
    else
    {
        qDebug() << "Serial port could not be opened. Error: " << serial.errorString();

        enableButtons(true);
    }
}

void MainWindow::on_downButton_pressed()
{
    serial.write("B 2000\r");

    qDebug() << "Going down...";
}

void MainWindow::on_upButton_pressed()
{
    serial.write("F 2000\r");
    qDebug() << "Going up...";
}

void MainWindow::on_rightButton_pressed()
{
    serial.write("L 2000\r");
    qDebug() << "Going right...";
}

void MainWindow::on_leftButton_pressed()
{
    serial.write("R 2000\r");
    qDebug() << "Going left...";
}

void MainWindow::on_zeroButton_pressed()
{
    serial.write("H\r");
}

void MainWindow::on_homeButton_pressed()
{
    serial.write("G 0 40000\r");
    qDebug() << "Going home...";
}

void MainWindow::on_connectButton_pressed()
{
    if (ui -> connectButton -> text() == "Connect" )
    {
        handleSerialCommunications();

        ui -> connectButton -> setText("Connected");
    }
}

void MainWindow::enableButtons(bool state)
{
    ui -> upButton -> setEnabled(state);
    ui -> zeroButton -> setEnabled(state);
    ui -> leftButton -> setEnabled(state);
    ui -> homeButton -> setEnabled(state);
    ui -> rightButton -> setEnabled(state);
    ui -> downButton -> setEnabled(state);
}
