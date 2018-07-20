#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QRect"

using namespace cv;

VideoCapture camera(0);

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&VideoTimer, SIGNAL(timeout()), this, SLOT(slotCompute()));

    VideoTimer.start(20);

    enableButtons(false);
    laplacianMode = false;
    guidesToggle = false;
}

MainWindow::~MainWindow()
{
    serialOptiScan.disconnect();
    serialZAxis.disconnect();

    delete ui;
}

void MainWindow::slotCompute()
{
    camera >> LiveImage; // MAT

    #ifdef RULER_GUIDE
        drawGuide(LiveImage, (IMAGE_HEIGHT / 4), 1, 10);
    #endif

    if (laplacianMode)
    {
        cvtColor(LiveImage, LapGrey, CV_BGRA2GRAY);
        Laplacian(LapGrey, LapMat, CV_16S, 3, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(LapMat, LapMat);

        focus = cv::sum( LapMat )[0];

        ui -> qualityLabel -> setText( "Quality: " + QString::number(focus) );

        if (guidesToggle)
        {
            cv::line(LapMat, cv::Point(0, IMAGE_HEIGHT / 2), cv::Point(IMAGE_WIDTH, IMAGE_HEIGHT / 2), cv::Scalar(255), GUIDES_SIZE);
            cv::line(LapMat, cv::Point((IMAGE_WIDTH / 2), 0), cv::Point((IMAGE_WIDTH / 2), IMAGE_HEIGHT), cv::Scalar(128), GUIDES_SIZE);
        }

        pixLiveImage = MatToQPixmap( LapMat );
    }
    else
    {
        ui -> qualityLabel -> setText( "Quality: Unavailable" );

        if (guidesToggle)
        {
            cv::line(LiveImage, cv::Point(0, IMAGE_HEIGHT / 2), cv::Point(IMAGE_WIDTH, IMAGE_HEIGHT / 2), cv::Scalar(255), GUIDES_SIZE);
            cv::line(LiveImage, cv::Point((IMAGE_WIDTH / 2), 0), cv::Point((IMAGE_WIDTH / 2), IMAGE_HEIGHT), cv::Scalar(128), GUIDES_SIZE);
        }

        pixLiveImage = MatToQPixmap( LiveImage );
    }

    ui -> videoLabel -> setPixmap( pixLiveImage );
}

void MainWindow::handleSerialCommunications()
{   
    serialOptiScan.setPortName("ttyUSB0"); // -> Might change

    if(serialOptiScan.open(QIODevice::ReadWrite))
    {
        if(!serialOptiScan.setBaudRate(QSerialPort::Baud9600))
        {
            qDebug() << serialOptiScan.errorString();
            return;
        }
        if(!serialOptiScan.setDataBits(QSerialPort::Data8))
        {
            qDebug() << serialOptiScan.errorString();
            return;
        }
        if(!serialOptiScan.setParity(QSerialPort::NoParity))
        {
            qDebug() << serialOptiScan.errorString();
            return;
        }
        if(!serialOptiScan.setStopBits(QSerialPort::OneStop))
        {
            qDebug() << serialOptiScan.errorString();
            return;
        }
        if(!serialOptiScan.setFlowControl(QSerialPort::NoFlowControl))
        {
            qDebug() << serialOptiScan.errorString();
            return;
        }

        enableButtons(true);
    }
    else
    {
        qDebug() << "Serial port could not be opened. Error: " << serialOptiScan.errorString();

        enableButtons(false);
    }

    /////////////////////////////////////////////////////////

    serialZAxis.setPortName("ttyACM1"); // -> Might change

    if(serialZAxis.open(QIODevice::ReadWrite))
    {
        if(!serialZAxis.setBaudRate(QSerialPort::Baud9600))
        {
            qDebug() << serialZAxis.errorString();
            return;
        }
        if(!serialZAxis.setDataBits(QSerialPort::Data8))
        {
            qDebug() << serialZAxis.errorString();
            return;
        }
        if(!serialOptiScan.setParity(QSerialPort::NoParity))
        {
            qDebug() << serialZAxis.errorString();
            return;
        }
        if(!serialOptiScan.setStopBits(QSerialPort::OneStop))
        {
            qDebug() << serialZAxis.errorString();
            return;
        }
        if(!serialOptiScan.setFlowControl(QSerialPort::NoFlowControl))
        {
            qDebug() << serialZAxis.errorString();
            return;
        }

        enableButtons(true);
    }
    else
    {
        qDebug() << "Serial port could not be opened. Error: " << serialZAxis.errorString();

        enableButtons(false);
    }
}

void MainWindow::on_downButton_pressed()
{
    serialOptiScan.write("B 1000\r");

    qDebug() << "Going down...";
}

void MainWindow::on_upButton_pressed()
{
    serialOptiScan.write("F 1000\r");
    qDebug() << "Going up...";
}

void MainWindow::on_rightButton_pressed()
{
    serialOptiScan.write("L 1000\r");
    qDebug() << "Going right...";
}

void MainWindow::on_leftButton_pressed()
{
    serialOptiScan.write("R 1000\r");
    qDebug() << "Going left...";
}

void MainWindow::on_zeroButton_pressed()
{
    serialOptiScan.write("H\r");
}

void MainWindow::on_homeButton_pressed()
{
    serialOptiScan.write("G 0 40000\r");
    qDebug() << "Going home...";
}

void MainWindow::on_gantryUpButton_pressed()
{
    serialZAxis.write("b\r");
}

void MainWindow::on_gantryDownButton_pressed()
{
    serialZAxis.write("f\r");
}

void MainWindow::on_connectButton_pressed()
{
    if (ui -> connectButton -> text() == "Connect" )
    {
        handleSerialCommunications();

        if (serialOptiScan.isOpen())
        {
            ui -> connectButton -> setText("Connected");
            ui -> connectButton -> setEnabled(false);
        }
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
    ui -> gantryUpButton -> setEnabled(state);
    ui -> gantryDownButton -> setEnabled(state);
}

void MainWindow::StackImagesFocusFast(void)
{
    Mat stackedImage(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, Scalar(0, 0, 0, 0));

    for(int i = 0; i < UN8NUMBEROFIMAGES; i++)
    {
        camera >> SavedImage[i];

        #ifdef MOTOR_SLEEP
        qSleep(20);
        #endif

        on_gantryUpButton_pressed();

        // serialOptiScan.write("R 95\r");
        // serialOptiScan.write("F 80\r");

        #ifdef MOTOR_SLEEP
        qSleep(800);
        #endif

        cvtColor(SavedImage[i], GreyImage[i], CV_BGRA2GRAY);
        Laplacian(GreyImage[i], LaplacianImage[i], CV_16S, 3, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(LaplacianImage[i], LaplacianImage[i]);

        cv::Rect roiScale(i * OFFSET_COLS, i * OFFSET_ROWS, IMAGE_WIDTH - (i * OFFSET_COLS), IMAGE_HEIGHT - (i * OFFSET_ROWS));

        SavedImage[i]( roiScale ).copyTo( SavedScaledImage[i] );
        testImage( SavedScaledImage[i] );
        qSleep(200);
        cv::resize(SavedScaledImage[i], SavedScaledImage[i], cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), 0, 0, INTER_NEAREST);
        testImage( SavedScaledImage[i] );
        qSleep(200);

        LaplacianImage[i]( roiScale ).copyTo( LaplacianScaledImage[i] );
        cv::resize(LaplacianScaledImage[i], LaplacianScaledImage[i], cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), 0, 0, INTER_NEAREST);
    }

    for(int i = 0; i < IMAGE_WIDTH; i += FOCUS_AREA_SIZE)
    {
        for(int j = 0; j < IMAGE_HEIGHT; j += FOCUS_AREA_SIZE)
        {
            bestFocus = 0.0;
            bestImage = 0;

            roi = cv::Rect(i, j, FOCUS_AREA_SIZE, FOCUS_AREA_SIZE);
            for(int k = 0; k < UN8NUMBEROFIMAGES; k++)
            {
                LaplacianScaledImage[k]( roi ).copyTo( focusArea );
                focus = cv::sum( focusArea )[0];
                if (focus > bestFocus)
                {
                    bestFocus = focus;
                    bestImage = k;
                }
            }
            SavedScaledImage[bestImage](roi).copyTo( stackedImage(roi) );
        }
    }

    testImage( stackedImage );
}

void MainWindow::pN(int num)
{
    qDebug() << QString::number(num);
}

void MainWindow::pM(QString mes)
{
    qDebug() << mes;
}

void MainWindow::pS(cv::Size s)
{
    qDebug() << "Width: " << QString::number(s.width) << "Height: " <<  QString::number(s.height);
}

void MainWindow::testImage(Mat img)
{
    QPixmap myQPixmap;
    myQPixmap = MatToQPixmap( img );
    ui -> testLabel -> setPixmap( myQPixmap );
}

void MainWindow::qSleep(int ms)
{
    QTime dieTime = QTime::currentTime().addMSecs(ms);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::on_testButton_pressed()
{
    StackImagesFocusFast();
}

void MainWindow::on_laplacianButton_pressed()
{
    if (ui -> laplacianButton -> text() == "Regular")
    {
        ui -> laplacianButton -> setText("Laplacian");
        laplacianMode = true;
    }
    else
    {
        ui -> laplacianButton -> setText("Regular");
        laplacianMode = false;
    }
}

void MainWindow::on_guidesButon_pressed()
{
    if (ui -> guidesButon -> text() == "Guides on")
    {
        ui -> guidesButon -> setText("Guides off");
        guidesToggle = false;
    }
    else
    {
        ui -> guidesButon -> setText("Guides on");
        guidesToggle = true;
    }
}

void MainWindow::drawGuide(Mat mat, int guideHeight, int guideThickness, int guideSpacing)
{
    cv::line(mat, cv::Point(0, IMAGE_HEIGHT / 2), cv::Point(IMAGE_WIDTH, IMAGE_HEIGHT / 2), cv::Scalar(0, 0, 255), guideThickness);

    int h = 0;
    bool guide = false;

    for(int i = 0; i < IMAGE_WIDTH; i += guideSpacing)
    {
        if (i == 0)
            continue;

        if (guide == false)
        {
            h = guideHeight;
            guide = true;
        }
        else
        {
            h = guideHeight / 2;
            guide = false;
        }

        if (i == (IMAGE_WIDTH / 2))
            cv::line(mat, cv::Point(i, (IMAGE_HEIGHT / 2) + h + 5), cv::Point(i, (IMAGE_HEIGHT / 2) - h - 5), cv::Scalar(0, 0, 255), 3);

        cv::line(mat, cv::Point(i, (IMAGE_HEIGHT / 2) + h), cv::Point(i, (IMAGE_HEIGHT / 2) - h), cv::Scalar(0, 0, 255), guideThickness);
    }
}
