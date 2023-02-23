#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "receivetime.h"

//General
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <fstream>

// Computer Vision
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

//Command Reception
QByteArray Placeholder;
QByteArray NameWriteArray;
int NameOrStepFlag = 0;

//OutputFiles
ofstream BackupFile;
string BackupFileName;
ofstream OutputFile;
string FileName;
string FileTime;
string FileLoc = "/home/pi/CPP_Tests/BEAM_Client_v013/Data/";
string Backup_File_Location = "/home/pi/CPP_Tests/BEAM_Server_v013/DataBackups/";

//Image Processing Delcaration
Mat src, gray_of_src, binary_of_gray, binary_plus_color, gray_plus_color, fail_img, imgroi, img;
Scalar col = Scalar(0, 255, 0); // green

//Open Video Stream
VideoCapture cap(0);

//Misc
QElapsedTimer etimer;
int PrevTime = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    timer = new QTimer(this);

    //Hide Text
    ui->HomeScreenText->setVisible(false);

    //Hide Control Box and Bideo
    ui->gridLayoutWidget->setVisible(true);
    ui->VideoWithControl->setVisible(true);

    //Hide Calibration Video
    ui->VideoWithoutControl ->setVisible(false);
}

void MainWindow::showEvent(QShowEvent *ev){
    QMainWindow::showEvent(ev);
    qDebug() << "MW";
    ClientSetUp();

    //Open Color Video for camera alignment
    RecordingTimer = 1000*60*10;
    OpenCamera();
}

void WriteToFile(){

    string FileLocAndName = FileLoc + FileName;
    OutputFile.open(FileLocAndName);
    if(OutputFile.is_open()){
        qDebug() << QString::fromStdString(FileName);
        qDebug() << "File Opened";
    }
    else{
        qDebug() << "File failed to open";
    }
    int i = 0;
    while(XPositionClient[i] != 0){
        if(i == 0){
            OutputFile << FileName << endl;
            OutputFile << FileTime << endl;
            OutputFile << "RightCalBefore,LeftCalBefore,TestData,RightCalAfter,LeftCalAfter" << endl;
            OutputFile << RightCalibrationBefore[i] << "," << LeftCalibrationBefore[i] << "," << XPositionClient[i] << "," << RightCalibrationAfter[i] << "," << LeftCalibrationAfter[i] << "," << endl;
        }
        else if(i > 0 && i < 90){
            OutputFile << RightCalibrationBefore[i] << "," << LeftCalibrationBefore[i] << "," << XPositionClient[i] << "," << RightCalibrationAfter[i] << "," << LeftCalibrationAfter[i] << "," << endl;
        }
        else{
            OutputFile << 0 << "," << 0 << "," << XPositionClient[i] << "," << 0 << "," << 0 << endl;
        }
        i++;
    }
    OutputFile.close();
}

void MainWindow::ClientSetUp(){

    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    qDebug() << "Slot Connect";
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    qDebug() << "Slot RR";

    qDebug() << "Connecting";

    usleep(1000000);

    socket->connectToHost(IP, 1234);

    qDebug() << "CTH";
    if(socket->waitForConnected(5000)){
        qDebug() << "WFC";
    }
    else{
        qDebug() << "Error: " << socket->errorString();
        close();
    }
    qDebug() << "CSU End";

}

void MainWindow::connected(){
    qDebug() << "Connected";
}

void MainWindow::readyRead(){
    qDebug() << "RR";

    if(NameOrStepFlag == 0){
        NameWriteArray = socket->readAll();
        NameOrStepFlag = 1;

        QString NameConvert = QString(NameWriteArray);
        FileName = NameConvert.toStdString();

        FileName.insert(0, "Client");
        BackupFileName = FileName;

        BackupFileName.insert(0, "Backup");

        qDebug() << "Client File Name: " << QString::fromStdString(FileName);
        qDebug() << "Client Backup Name: " << QString::fromStdString(BackupFileName);


    }
    else{
        Placeholder = socket->readAll();
        Step = Placeholder.toInt();
        qDebug() << "Step: " << Placeholder;
        Distro();
    }
}

void MainWindow::Distro(){

    ui->HomeScreenText->setVisible(false);

    if(Step == 1){
        DisplaySelector = 0;
        DataSavingFlag = 0;
        RecordingTimer = SetUpTime;

        ui->gridLayoutWidget->setVisible(true);
        ui->VideoWithControl->setVisible(true);
    }
    else if(Step == 2){
        DisplaySelector = 1;
        DataSavingFlag = 1;
        RecordingTimer = CalibrationTime;
        ui->VideoWithoutControl->setVisible(true);
    }
    else if(Step == 3){
        DisplaySelector = 1;
        DataSavingFlag = 2;
        RecordingTimer = CalibrationTime;
        ui->VideoWithoutControl->setVisible(true);
    }
    else if(Step == 4){
        ReceiveTime RT;
        RT.setModal(true);
        RT.exec();

        DisplaySelector = 0;
        DataSavingFlag = 3;
        RecordingTimer = Test_Time;

        ui->gridLayoutWidget->setVisible(true);
        ui->VideoWithControl->setVisible(true);
    }
    else if(Step == 10){
        close();
    }
    else{
        Step = 0;
    }

    OpenCamera();

}

void MainWindow::OpenCamera(){
    TimeCount = 0;
    if(!cap.isOpened())  // Check if we succeeded
    {
        cout << "camera is not open" << endl;
    }
    else
    {
        cout << "camera is open" << endl;
        connect(timer, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
        qDebug() << "Slot Connected";
        timer->start(20);
        qDebug() << "Timer Started";
        etimer.start();
        qDebug() << "ETimer Started";
    }
}

void MainWindow::UpdateFrame(){

    if(X+Width > 639) Width = 639-X;
    if(Width <= 0) Width = 1;
    if(Y+Height > 479) Height = 479-Y;
    if(Height <= 0) Height = 1;

    cap >> src;

    Rect roi(X, Y, Width, Height);
    cvtColor(src, gray_of_src, COLOR_BGR2GRAY); // color to gray
    threshold(gray_of_src, binary_of_gray, thresh_val, thresh_max_val, thresh_type); //gray to binary
    imgroi = binary_of_gray(roi);

    cvtColor(binary_of_gray, binary_plus_color, COLOR_GRAY2RGB); // enable color on binary

    vector<Vec3f> circles;
    HoughCircles(imgroi, circles, HOUGH_GRADIENT, 1, 1000, CED, Cent_D, max_rad-2, max_rad);
    Vec3i c;
    for( size_t i = 0; i < circles.size(); i++ ){
        c = circles[i];
    }

    X_Point = c[0]+X;
    Y_Point = c[1]+Y;
    Radius = c[2];

    //Draw Circles and Box On Black and White
    circle(binary_plus_color, Point(X_Point, Y_Point), 1, col,1,LINE_8);
    circle(binary_plus_color, Point(X_Point, Y_Point), Radius, col,1,LINE_8);
    rectangle(binary_plus_color, Point(X, Y),Point(X+Width, Y+Height),col,1,LINE_8); // bounding box

    if(DisplaySelector == 0){
        if(ColorOrBW == 0){
            ui->VideoWithControl->setPixmap(QPixmap::fromImage(QImage((unsigned char*) src.data, src.cols, src.rows, src.step, QImage::Format_RGB888)));
            //Color Video
        }
        else{
            ui->VideoWithControl->setPixmap(QPixmap::fromImage(QImage((unsigned char*) binary_plus_color.data, binary_plus_color.cols, binary_plus_color.rows, binary_plus_color.step, QImage::Format_RGB888)));
            //Video with the controls
        }
    }
    else{
        ui->VideoWithoutControl->setPixmap(QPixmap::fromImage(QImage((unsigned char*) binary_plus_color.data, binary_plus_color.cols, binary_plus_color.rows, binary_plus_color.step, QImage::Format_RGB888)));
        //Video without control
    }


    if(DataSavingFlag == 1 && TestCompleteFlag == 0){
        RightCalibrationBefore[TimeCount] = X_Point;
        if(TimeCount == 0){
            BackupFile.open(Backup_File_Location + BackupFileName);
            if(BackupFile.is_open()){
                qDebug() << QString::fromStdString(BackupFileName);;
            }
            else{
                qDebug() << "Backup File not open";
                cerr << "Error: " << strerror(errno);
            }
            BackupFile << FileName << endl;
            BackupFile << FileTime << endl;
            BackupFile << "\n\n" << "RightCalBefore" << endl;
            BackupFile << X_Point << endl;
        }
        else{
            BackupFile << X_Point << endl;
        }
    }
    else if(DataSavingFlag == 2 && TestCompleteFlag == 0){
        LeftCalibrationBefore[TimeCount] = X_Point;
        if(TimeCount == 0){
            BackupFile << "\n\n" << "LeftCalBefore" << endl;
            BackupFile << X_Point << endl;
        }
        else{
            BackupFile << X_Point << endl;
        }
    }
    else if(DataSavingFlag == 3 && TestCompleteFlag == 0){
        XPositionClient[TimeCount] = X_Point;
        if(TimeCount == 0){
            BackupFile << "\n\n" << "TestData" << endl;
            BackupFile << X_Point << endl;
        }
        else{
            BackupFile << X_Point << endl;
        }
    }
    else if(DataSavingFlag == 1 && TestCompleteFlag == 1){
        RightCalibrationAfter[TimeCount] = X_Point;
        if(TimeCount == 0){
            BackupFile << "\n\n" << "RightCalAfter" << endl;
            BackupFile << X_Point << endl;
        }
        else{
            BackupFile << X_Point << endl;
        }
    }
    else if(DataSavingFlag == 2 && TestCompleteFlag == 1){
        LeftCalibrationAfter[TimeCount] = X_Point;
        if(TimeCount == 0){
            BackupFile << "\n\n" << "LeftCalAfter" << endl;
            BackupFile << X_Point << endl;
        }
        else{
            BackupFile << X_Point << endl;
        }
    }
    else{
        //Do nothing
    }
    if(etimer.elapsed() >= RecordingTimer){
        timer->stop();
        if(ColorOrBW == 0){
            ColorOrBW = 1;
        }
        if(DataSavingFlag == 3){
            TestCompleteFlag = 1;
        }
        if(DataSavingFlag == 2 && TestCompleteFlag == 1){
            BackupFile.close();
            WriteToFile();
        }
        ReturntoHomeScreen();
        return;
    }
    TimeCount++;
    qDebug() << "Client Diff: " << etimer.elapsed() - PrevTime;
    qDebug() << "Client Time: " << etimer.elapsed();
    qDebug() << "Client Count: " << TimeCount;
    PrevTime = etimer.elapsed();
}

void MainWindow::ReturntoHomeScreen(){

    //Stop Timer
    timer->stop();

    //Show buttons
    ui->HomeScreenText->setVisible(true);

    //Disable Video & Controls
    ui->gridLayoutWidget->setVisible(false);
    ui->VideoWithControl->setVisible(false);

    //Disable Calibration Video
    ui->VideoWithoutControl->setVisible(false);
}

void MainWindow::on_ThresholdSlider_valueChanged(int Threshold)
{
    ui->ThresholdDisplay->display(Threshold);
    thresh_val = Threshold;
}

void MainWindow::on_CircleSizeSlider_valueChanged(int rad)
{
    ui->CircleSizeDisplay->display(rad);
    max_rad = rad;
}

void MainWindow::on_BoxXSlider_valueChanged(int BoxX)
{
    ui->BoxXDisplay->display(BoxX);
    X = BoxX;
}

void MainWindow::on_BoxYSlider_valueChanged(int BoxY)
{
    ui->BoxYDisplay->display(BoxY);
    Y = BoxY;
}

void MainWindow::on_BoxHSlider_valueChanged(int BoxH)
{
    ui->BoxHDisplay->display(BoxH);
    Height = BoxH;
}

void MainWindow::on_BoxWSlider_valueChanged(int BoxW)
{
    ui->BoxWDisplay->display(BoxW);
    Width = BoxW;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_CloseControlPanel_clicked()
{
    if(ColorOrBW == 0){
        ColorOrBW = 1;
    }
    if(DataSavingFlag == 3){
        TestCompleteFlag = 1;
    }
    ReturntoHomeScreen();
}
