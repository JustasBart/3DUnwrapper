#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QRect"

using namespace cv;

static VideoCapture camera(0);

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    camera.set(cv::CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT);
    
    connect(&VideoTimer, SIGNAL(timeout()), this, SLOT(slotCompute()));
    
    VideoTimer.start(20);
    
    enableButtons(false);
    laplacianMode = false;
    guidesToggle = false;
    
    focus_area_size = 4;

    upperFocus = 0;
    lowerFocus = 0;
    cancelStack = false;
}

MainWindow::~MainWindow()
{
    cancelStack = true;
    enableButtons(false);

    serialOptiScan.disconnect();
    serialOmni.disconnect();
    
    delete ui;
}

void MainWindow::slotCompute()
{
    camera >> LiveImage; // MAT
    
#ifdef RULER_GUIDE
    drawGuide(LiveImage, (IMAGE_HEIGHT / 12), 5, 50);
#endif
    
    if (laplacianMode)
    {
        cvtColor(LiveImage, LapGrey, CV_BGRA2GRAY);
        Laplacian(LapGrey, LapMat, CV_8U, 1, 5, 0, BORDER_ISOLATED);
        convertScaleAbs(LapMat, LapMat);
        
        cv::GaussianBlur( LapMat, LapMat, cv::Point(11, 11), 0 );
        
        // processHeight( LapMat[0] );
        
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
    
    if (serialOmni.isOpen())
    {
        QString message;
        
        message = serialOmni.readLine();
        
        if (message.length() > 0)
        {
            ui -> serialLabel -> setText( message );
            
            message = "";
        }
    }
}

void MainWindow::handleSerialCommunications()
{   
    serialOptiScan.setPortName(OPTI_SCAN_SERIAL); // -> Might change
    
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
    
    serialOmni.setPortName(OMNI_SERIAL); // -> Might change
    
    if(serialOmni.open(QIODevice::ReadWrite))
    {
        if(!serialOmni.setBaudRate(QSerialPort::Baud115200))
        {
            qDebug() << serialOmni.errorString();
            return;
        }
        if(!serialOmni.setDataBits(QSerialPort::Data8))
        {
            qDebug() << serialOmni.errorString();
            return;
        }
        if(!serialOptiScan.setParity(QSerialPort::NoParity))
        {
            qDebug() << serialOmni.errorString();
            return;
        }
        if(!serialOptiScan.setStopBits(QSerialPort::OneStop))
        {
            qDebug() << serialOmni.errorString();
            return;
        }
        if(!serialOptiScan.setFlowControl(QSerialPort::NoFlowControl))
        {
            qDebug() << serialOmni.errorString();
            return;
        }
        
        enableButtons(true);
    }
    else
    {
        qDebug() << "Serial port could not be opened. Error: " << serialOmni.errorString();
        
        enableButtons(false);
    }
}

void MainWindow::on_leftButton_pressed()
{

    moveGantry( GantryPosition::Left, ui -> moveByDistance -> value() );
}
void MainWindow::on_rightButton_pressed()
{
    moveGantry(GantryPosition::Right, ui -> moveByDistance -> value() );
}
void MainWindow::on_upButton_pressed()
{
    moveGantry(GantryPosition::Up, ui -> moveByDistance -> value() );
}
void MainWindow::on_downButton_pressed()
{
    moveGantry(GantryPosition::Down, ui -> moveByDistance -> value() );
}

void MainWindow::on_zeroButton_pressed()
{
    serialOptiScan.write("H\r");
    qSleep(100);
}

void MainWindow::on_homeButton_pressed()
{
    serialOptiScan.write(("L " + QString::number(findMoveZoomRatio()) + " \r").toStdString().c_str());
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
    ui -> minFocus -> setEnabled(state);
    ui -> maxFocus -> setEnabled(state);
    
    ui -> lowerBoundry -> setEnabled(state);
    ui -> upperBoundry -> setEnabled(state);
}

void MainWindow::StackImagesFocusFast(Mat threeDimentional)
{
    focusBy = lowerFocus < upperFocus ? ((upperFocus - lowerFocus) / UN8NUMBEROFIMAGES) : ((lowerFocus - upperFocus) / UN8NUMBEROFIMAGES);
    focusHeight = lowerFocus < upperFocus ? lowerFocus : upperFocus;
    
    for(int i = 0; i < UN8NUMBEROFIMAGES; i++)
    {
        if (!cancelStack)
        {
            zoomOutHeight();
            qSleep(150);

            camera >> SavedImage[i];
            focusHeight += focusBy;

            qDebug() << QString::number(focusHeight);

            cvtColor( SavedImage[i], GreyImage[i], CV_BGRA2GRAY );
            Laplacian( GreyImage[i], LaplacianImage[i], CV_8U, 1, 5, 0, BORDER_ISOLATED );
            convertScaleAbs( LaplacianImage[i], LaplacianImage[i] );

            cv::GaussianBlur( LaplacianImage[i], LaplacianImage[i], cv::Point(11, 11), 0 );
        }
        else
            return;
    }
    
    if (!cancelStack)
    {
        processHeight( LaplacianImage, threeDimentional );
        cv::medianBlur(threeDimentional, threeDimentional, 31);
    }
}

void MainWindow::zoomOutHeight(void)
{
    QString zoomOutMessage = "F_DIR " + QString::number( focusHeight ) + " \n";
    serialOmni.write( zoomOutMessage.toStdString().c_str() );
    qSleep(100);

    if (findFocusLevel() != focusHeight)
    {
        pM("The focus level could not be set. Re-sending height level!");
        serialOmni.write( zoomOutMessage.toStdString().c_str() );
        qSleep(100);

        if (findFocusLevel() != focusHeight)
        {
            pM("The focus level could not be set for the second time!!!");
        }
    }
}

void MainWindow::processHeight(Mat heigthMat[], Mat finalMat)
{
    int heightWeight = 255 / UN8NUMBEROFIMAGES;
    
    for(int i = 0; i < IMAGE_WIDTH; i += focus_area_size)
    {
        for(int j = 0; j < IMAGE_HEIGHT; j += focus_area_size)
        {
            bestFocus = 0.0;
            bestImage = 0;
            
            for(int k = 0; k < UN8NUMBEROFIMAGES; k++)
            {
                roi = cv::Rect(i, j, focus_area_size, focus_area_size);
                
                for(int n = 0; n < 500; n++)
                {
                    if (!cancelStack)
                    {
                        heigthMat[k]( roi ).copyTo( focusArea );
                        focus = cv::sum( focusArea )[0];

                        if (focus > FOCUS_ENERGY) // Pass filter
                        {
                            if (focus > bestFocus && checkLocalEnergy(heigthMat[k], roi, n))
                            {
                                bestFocus = focus;
                                bestImage = k;

                                break;
                            }
                        }
                    }
                    else
                        return;
                }
            }
            
            finalMat(roi) = Scalar(bestImage * heightWeight);
        }
    }
}

bool MainWindow::checkLocalEnergy(Mat piece, cv::Rect region, int position)
{
    if (region.x - position >= 0 && region.y - position >= 0)
    {
        if (region.x + focus_area_size + position <= IMAGE_WIDTH && region.y + focus_area_size + position <= IMAGE_HEIGHT)
        {
            region.x -= position;
            region.y -= position;
            
            region.width += position * 2;
            region.height += position * 2;
            
            Mat tempMat(region.height, region.width, CV_8UC1, Scalar(0));
            double newFocus;
            
            piece( region ).copyTo( tempMat );
            newFocus = cv::sum( tempMat )[0];
            
            return newFocus > (FOCUS_ENERGY + FOCUS_ENERGY * 0.2) ? true : false;
        }
    }
    
    return false;
}

int MainWindow::findFocusLevel()
{
    int replyValues[REPLY_ATTEMPTS];
    int goodValue;
    QString reply;

    for (int i = 0; i < REPLY_ATTEMPTS; i++)
    {
        serialOmni.write("F_READ \n");
        if (serialOmni.waitForReadyRead(2000))
        {
            reply = serialOmni.readLine();
            replyValues[i] = reply.toInt();
            qSleep(100);
        }
        else
            ui -> serialLabel -> setText("FAILLED TO GET A REPLY!");
    }

    for (int i = 0; i < REPLY_ATTEMPTS; i++)
    {
        if (replyValues[i] == replyValues[i + 1])
        {
            goodValue = replyValues[i];
        }
    }
    
    return goodValue;
}

void MainWindow::pN(double num)
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
    qSleep(1000);
    ui -> testLabel -> setPixmap( myQPixmap );
}

void MainWindow::qSleep(int ms)
{
    QTime dieTime = QTime::currentTime().addMSecs(ms);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}



double MainWindow::findMoveZoomRatio(void)
{
    double replyValues[REPLY_ATTEMPTS];
    double goodValue = 0.0;

    QString reply;

    for (int i = 0; i < REPLY_ATTEMPTS; i++)
    {
        serialOmni.write("PIXEL2MM \n");
        if (serialOmni.waitForReadyRead(2000))
        {
            reply = serialOmni.readLine();
            replyValues[i] = reply.toDouble();
            qSleep(100);
        }
        else
            ui -> serialLabel -> setText("FAILLED TO GET A REPLY!");
    }

    for (int i = 0; i < REPLY_ATTEMPTS; i++)
    {
        if (fabs(replyValues[i] - replyValues[i + 1]) <= 0.1 * fabs(replyValues[i]))
        {
            goodValue = replyValues[i];
        }
    }

    return goodValue;
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
            cv::line(mat, cv::Point(i, (IMAGE_HEIGHT / 2) + h + 5), cv::Point(i, (IMAGE_HEIGHT / 2) - h - 5), cv::Scalar(255, 255, 255), 3);
        else
            cv::line(mat, cv::Point(i, (IMAGE_HEIGHT / 2) + h), cv::Point(i, (IMAGE_HEIGHT / 2) - h), cv::Scalar(0, 0, 255), guideThickness);
    }
}

void MainWindow::on_lowerBoundry_pressed()
{
    lowerFocus = findFocusLevel();
    
    if (lowerFocus == upperFocus)
    {
        ui -> lBoundLabel -> setText("INVALID");
        lowerFocus = 0;
        return;
    }
    
    ui -> lBoundLabel -> setText( QString::number(lowerFocus) );
    stackAllow[0] = true;
    
    checkStackStatus();
}

void MainWindow::on_upperBoundry_pressed()
{
    upperFocus = findFocusLevel();
    
    if (upperFocus == lowerFocus)
    {
        ui -> uBoundLabel -> setText("INVALID");
        upperFocus = 1;
        return;
    }
    
    ui -> uBoundLabel -> setText( QString::number(upperFocus) );
    stackAllow[1] = true;
    
    checkStackStatus();
}

void MainWindow::checkStackStatus(void)
{
    if (stackAllow[0] && stackAllow[1])
        ui -> stackButton -> setEnabled(true);
    else
        ui -> stackButton -> setEnabled(false);
}

void MainWindow::moveGantry(GantryPosition instruction, double millimeter)
{
    QString sendCommand;

    switch(instruction)
    {
    case GantryPosition::Left:
    {
        sendCommand = "R " + QString::number( static_cast<int>(millimeter * STEP_MM_MULTIPLIER) ) + "\r";
        qDebug() << "Going left by " + QString::number(millimeter) + "mm... " + QString::number(static_cast<int>(millimeter * STEP_MM_MULTIPLIER)) + " Steps.";
        break;
    }
    case GantryPosition::Right:
    {
        sendCommand = "L " + QString::number( static_cast<int>(millimeter * STEP_MM_MULTIPLIER) ) + "\r";
        qDebug() << "Going right by " + QString::number(millimeter) + "mm... " + QString::number(static_cast<int>(millimeter * STEP_MM_MULTIPLIER)) + " Steps.";
        break;
    }
    case GantryPosition::Up:
    {
        sendCommand = "F " + QString::number( static_cast<int>(millimeter * STEP_MM_MULTIPLIER) ) + "\r";
        qDebug() << "Going up by " + QString::number(millimeter) + "mm... " + QString::number(static_cast<int>(millimeter * STEP_MM_MULTIPLIER)) + " Steps.";
        break;
    }
    case GantryPosition::Down:
    {
        sendCommand = "B " + QString::number( static_cast<int>(millimeter * STEP_MM_MULTIPLIER) ) + "\r";
        qDebug() << "Going down by " + QString::number(millimeter) + "mm... " + QString::number(static_cast<int>(millimeter * STEP_MM_MULTIPLIER)) + " Steps.";
        break;
    }
    }

    serialOptiScan.write(sendCommand.toStdString().c_str());
}

void MainWindow::on_minFocus_pressed()
{
    serialOmni.write("F_DIR 4096 \n");

    if (findFocusLevel() != 4096)
    {
        pM("The minimum focus level could not be set. Re-sending the mininmum focus level!");
        serialOmni.write("F_DIR 4096 \n");
        qSleep(100);

        if (findFocusLevel() != 4096)
        {
            pM("The minimum focus level could not be set for the second time.");
        }
    }
}

void MainWindow::on_maxFocus_pressed()
{
    serialOmni.write("F_DIR 53248 \n");

    if (findFocusLevel() != 53248)
    {
        pM("The maximum focus level could not be set. Re-sending the maximum focus level!");
        serialOmni.write("F_DIR 53248 \n");

        if (findFocusLevel() != 53248)
        {
            pM("The maximum focus level could not be set for the second time.");
        }
    }
}

void MainWindow::on_stackButton_pressed()
{
    if (ui -> stackButton -> text() == "StackImages")
    {
        cancelStack = false;
        enableButtons(false);
        ui -> stackButton -> setText("Cancel");

        int x, y;

        int wTimes = ui -> widthSpinBox -> value();
        int hTimes = ui -> heightSpinBox -> value();

        on_minFocus_pressed();

        double zRatio = findMoveZoomRatio();


        double xMoveBy = zRatio * IMAGE_WIDTH;
        double yMoveBy = zRatio * IMAGE_HEIGHT;

        Mat full3DMat(hTimes * IMAGE_HEIGHT, wTimes * IMAGE_WIDTH, CV_8UC1, Scalar(0));

        for (y = 0; y < hTimes; y++)
        {
            for (x = 0; x < wTimes; x++)
            {
                if(wTimes > 1 && x > 0)
                {
                    moveGantry(GantryPosition::Right, xMoveBy);
                    qSleep(2000);
                }

                cv::Rect roi(x * IMAGE_WIDTH, y * IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT);
                Mat partOf3DMat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1, Scalar(0));

                StackImagesFocusFast( partOf3DMat );
                partOf3DMat.copyTo( full3DMat(roi) );

                testImage(full3DMat);
            }

            if (hTimes > 1 && (y + 1) != hTimes)
            {
                if (wTimes > 1)
                {
                    moveGantry(GantryPosition::Left, (wTimes - 1) * xMoveBy );
                    qSleep(500);
                }
                moveGantry(GantryPosition::Down, yMoveBy );
                qSleep(7000);
            }
        }

        testImage(full3DMat);

        QTime currentTime = QTime::currentTime();
        QString label = "../../DepthMap_" + QString::number(currentTime.minute()) + "_" + QString::number(currentTime.second()) + ".jpg";
        imwrite(label.toStdString().c_str(), full3DMat);

        cancelStack = true;
        enableButtons(true);
        ui -> stackButton -> setText("StackImages");
    }
    else
    {
        cancelStack = true;
        enableButtons(true);
        ui -> stackButton -> setText("StackImages");
    }
}
