#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "settime.h"

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

//File Naming Declarations
ofstream BackupFile;
string BackupFileName;
string FileName;
string FileTime;
ofstream OutputFile;
int FileNameLen = 0;
QByteArray NameWriteArray;
string File_Location = "/home/pi/CPP/BEAM_Server_v013/Data/";
string Backup_File_Location = "/home/pi/CPP/BEAM_Server_v013/DataBackups/";

//Step Sending Declarations
QByteArray SteptoWrite;
int NameOrStepFlag = 0;

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

    Server = new QTcpServer(this);
    Timer = new QTimer(this);

    //Connect server to slot so it gets called when the new connection comes in
    connect(Server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    //Set up server and check to ensure it worked
    if(!Server->listen(QHostAddress::Any, 1234)){
        qDebug() << "Server could not start";
    }
    else{
        qDebug() << "Server started";
    }

    //Disable buttons
    ui->CalibrateRight->setEnabled(false);
    ui->CalibrateLeft->setEnabled(false);
    ui->Diagnostic->setEnabled(false);
    ui->Therapy->setEnabled(false);

    //Hide controls and camera window
//    ui->gridLayoutWidget_2->setVisible(false);
//    ui->VideoWithControl->setVisible(false);

    ui->gridLayoutWidget->setVisible(false);

    //Hide camera window
    ui->VideoWithoutControls->setVisible(false);


}

void MainWindow::newConnection()
{
    Socket = Server->nextPendingConnection();
    qDebug() << "Main Connected";
    //Call function to set the file name
    setFileName();
}

void MainWindow::writeToSocket(){
    if(NameOrStepFlag == 0){
        QString QtoWrite = QString::fromStdString(FileName);
        NameWriteArray = QtoWrite.toUtf8();
        qDebug() << NameWriteArray;
        Socket->write(NameWriteArray);
        NameOrStepFlag = 1;
    }
    else{
        Socket->write(SteptoWrite);
    }
    Socket->flush();
}

void writeToFile(){
    string FileNamewLoc = File_Location + FileName;
    OutputFile.open(FileNamewLoc);
    if(OutputFile.is_open()){
        qDebug() << QString::fromStdString(FileName);
        qDebug() << "File Opened";
    }
    else{
        qDebug() << "File failed to open";
    }
    int i = 0;
    while(XPositionServer[i] != 0){
        if(i == 0){
            OutputFile << FileName << endl;
            OutputFile << FileTime << endl;
            OutputFile << "RightCalBefore,LeftCalBefore,TestData,RightCalAfter,LeftCalAfter" << endl;
            OutputFile << RightCalibrationBefore[i] << "," << LeftCalibrationBefore[i] << "," << XPositionServer[i] << "," << RightCalibrationAfter[i] << "," << LeftCalibrationAfter[i] << "," << endl;
        }
        else if(i > 0 && i < 90){
            OutputFile << RightCalibrationBefore[i] << "," << LeftCalibrationBefore[i] << "," << XPositionServer[i] << "," << RightCalibrationAfter[i] << "," << LeftCalibrationAfter[i] << "," << endl;
        }
        else{
            OutputFile << 0 << "," << 0 << "," << XPositionServer[i] << "," << 0 << "," << 0 << endl;
        }
        i++;
    }
    OutputFile.close();
}

string removeSpacefromFilename(string str){
    cout << "str: " << str << endl;
    while(str.find(" ") != std::string::npos){
        cout << "sf: " << str.find(" ") << endl;
        str.erase(str.find(" "), 1);
        cout << "str: " << str << endl;
    }
    return(str);
}

void MainWindow::setFileName(){

    string Convert_stdout;
    string Path_File;
    string File_Extension = ".txt";

    time_t now = time(0);
    string dt = ctime(&now); // Get Time
    dt = dt.substr(0, dt.length()-1); //Remove trailing new line

    int MonthStart = dt.find(" "); //First space denotes start of Month
    string Month = dt.substr(MonthStart + 1, dt.find(" ", MonthStart+1) - 1 - MonthStart);//Next space dentoes end of month

    int DayStart = dt.find(" ", MonthStart+1); // Second space denotes start of day number
    string Day = dt.substr(DayStart+1, dt.find(" ", DayStart + 2) - (DayStart+1)); //Next space denotes end of day
    Day = removeSpacefromFilename(Day);

    int TimeStart = dt.find(" ", DayStart + 2);//Space after day dentoes start of time
    string Time = dt.substr(TimeStart+1, dt.find(" ", TimeStart+3) - 1 - TimeStart);//Next space denotes end of time
    Time = Time.erase(Time.find(":"),1); //Remove first :
    Time = Time.erase(Time.find(":"), Time.length()-Time.find(":")); //Remove second :

    string Year = dt.substr(dt.length()-4); //Year is last 4 character

    FileTime = Day+Month+Year+Time;
    cout << FileTime << endl;

    int NameBufSize = 128;
    char NameBuf[NameBufSize];
    string Syscall = "/home/pi/CPP/\.\/KB_Call.sh";
    //string Syscall = "/home/four/Dropbox/School/PhD/PhD\\ Work/Codes/\.\/KB_Call.sh";
    FILE *Name = popen(Syscall.c_str(), "r");

    while(fgets(NameBuf, NameBufSize, Name) != NULL){
        cout << NameBuf << strlen(NameBuf) << endl;
        if(strlen(NameBuf) > 1){
            FileName = NameBuf + File_Extension;
            FileName.replace(FileName.find("\n"), 1, "");
            cout << "FileName: " << FileName << endl;
            Path_File = File_Location + FileName;
            cout << "Path & File: " << Path_File << endl;
        }
    }
    cout << "Name Length: " << strlen(NameBuf) << endl;

    FileName.insert(FileName.find(".txt"), FileTime);
    writeToSocket();

    //FileNameLen = strlen(NameBuf) - 1;
    FileName.insert(0, "Server");
    BackupFileName = FileName;

    BackupFileName.insert(0, "Backup");
    cout << "Backup Name is " << BackupFileName << endl;

    //Open Color Video for camera alignment
    RecordingTimer = 1000*60*10;
    openCamera();
}

void MainWindow::openCamera(){
    TimeCount = 0;
    if(!cap.isOpened())  // Check if we succeeded
    {
        cout << "camera is not open" << endl;
    }
    else
    {
        cout << "camera is open" << endl;
        connect(Timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
        qDebug() << "Slot Connected";
        Timer->start(20);
        qDebug() << "Timer Started";
        etimer.start();
        qDebug() << "ETimer Started";
    }
}

void MainWindow::updateFrame(){

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
        ui->VideoWithoutControls->setPixmap(QPixmap::fromImage(QImage((unsigned char*) binary_plus_color.data, binary_plus_color.cols, binary_plus_color.rows, binary_plus_color.step, QImage::Format_RGB888)));
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
        XPositionServer[TimeCount] = X_Point;
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
        Timer->stop();
        if(ColorOrBW == 0){
            ColorOrBW = 1;
        }
        if(DataSavingFlag == 3){
            TestCompleteFlag = 1;
            qDebug() << "Test Done";
        }
        if(DataSavingFlag == 2 && TestCompleteFlag == 1){
            BackupFile.close();
            qDebug() << "All Done";
            writeToFile();
            qDebug() << "Output Written";
        }
        returntoHomeScreen();
        return;
    }
    TimeCount++;
    qDebug() << "Server Diff: " << etimer.elapsed() - PrevTime;
    qDebug() << "Server Time: " << etimer.elapsed();
    qDebug() << "Server Count: " << TimeCount;
    PrevTime = etimer.elapsed();
}

void MainWindow::returntoHomeScreen(){

    //Stop timer
    Timer->stop();

    //Show buttons
    ui->gridLayoutWidget->setVisible(true);

    //Disable Video & Controls
    ui->VideoWithControl->setVisible(false);
    ui->gridLayoutWidget_2->setVisible(false);

    //Disable Calibration Video
    ui->VideoWithoutControls->setVisible(false);
}

void MainWindow::on_ThresholdSlider_valueChanged(int Threshold)
{
    ui->ThresholdDisplay->display(Threshold);
    thresh_val = Threshold;
}

void MainWindow::on_CircleSizeSlider_valueChanged(int Radius)
{
    ui->CircleSizeDisplay->display(Radius);
    max_rad = Radius;
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

void MainWindow::on_CloseControls_clicked()
{
    if(ColorOrBW == 0){
        ColorOrBW = 1;
    }
    if(DataSavingFlag == 3){
        TestCompleteFlag = 1;
    }
    returntoHomeScreen();
}

void MainWindow::on_TrackingSetUp_clicked()
{
    //Ui Elements
    ui->gridLayoutWidget->setVisible(false); //Hide Buttons
    ui->gridLayoutWidget_2->setVisible(true); //Show Controls
    ui->VideoWithControl->setVisible(true); //Show Video

    //Set slider values
    ui->ThresholdSlider->setValue(thresh_val);
    ui->CircleSizeSlider->setValue(max_rad);
    ui->BoxXSlider->setValue(X);
    ui->BoxYSlider->setValue(Y);
    ui->BoxHSlider->setValue(Height);
    ui->BoxWSlider->setValue(Width);

    Step = 1;
    SteptoWrite.setNum(Step);
    writeToSocket();

    DisplaySelector = 0;
    DataSavingFlag = 0;
    RecordingTimer = SetUpTime;
    openCamera();
    ui->CalibrateRight->setEnabled(true);
}

void MainWindow::on_CalibrateRight_clicked()
{
    ui->gridLayoutWidget->setVisible(false);
    ui->VideoWithoutControls->setVisible(true);

    Step = 2;
    SteptoWrite.setNum(Step);
    writeToSocket();

    DisplaySelector = 1;
    DataSavingFlag = 1;
    RecordingTimer = CalibrationTime;
    openCamera();

    ui->CalibrateLeft->setEnabled(true);

}

void MainWindow::on_CalibrateLeft_clicked()
{
    ui->gridLayoutWidget->setVisible(false);
    ui->VideoWithoutControls->setVisible(true);

    Step = 3;
    SteptoWrite.setNum(Step);
    writeToSocket();

    DisplaySelector = 1;
    DataSavingFlag = 2;
    RecordingTimer = CalibrationTime;
    openCamera();

    ui->Diagnostic->setEnabled(true);

}

void MainWindow::on_Diagnostic_clicked()
{
    //Ui Elements
    ui->gridLayoutWidget->setVisible(false); //Hide Buttons
    ui->gridLayoutWidget_2->setVisible(true); //Show Controls
    ui->VideoWithControl->setVisible(true); //Show Video

    //Set slider values
    ui->ThresholdSlider->setValue(thresh_val);
    ui->CircleSizeSlider->setValue(max_rad);
    ui->BoxXSlider->setValue(X);
    ui->BoxYSlider->setValue(Y);
    ui->BoxHSlider->setValue(Height);
    ui->BoxWSlider->setValue(Width);

    Step = 4;
    SteptoWrite.setNum(Step);
    writeToSocket();

    SetTime ST;
    ST.setModal(true);
    ST.exec();

    DisplaySelector = 0;
    DataSavingFlag = 3;
    RecordingTimer = Test_Time;
    openCamera();
}

void MainWindow::on_Quit_clicked()
{
    Step = 10;
    SteptoWrite.setNum(Step);
    writeToSocket();

    close();
}

MainWindow::~MainWindow()
{
    delete ui;
}
