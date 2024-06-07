#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "settime.h"
#include "BEAM_Functions.h"

#include "stdio.h"
#include "iostream"
#include "stdio.h"
#include "unistd.h"
#include "string"
#include "libuvc/libuvc.h"
#include "turbojpeg.h"
#include "math.h"
#include "chrono"
#include "list"
#include "fstream"

// Computer Vision
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

//Namespaces
using namespace std;
using namespace cv;

//Instantiating Classes
Camera rightEyeCam;
Camera leftEyeCam;
Frame frameProc;

//Elapsed time to include in data
QElapsedTimer elapsed_timer;

void MainWindow::alignCameras(){
    ColorOrBW = 0;
    RecordingTimer = SetUpTime;
    startCamera();
}

void MainWindow::initSystem(){
    printf("\n\n\n");
    vector<string> camNames(3);
    camNames = getCameraNames();
//    cout << camNames[0] << "\n"  << camNames[1] << "\n"  << camNames[2] << endl;
    for (int i=0; i<2; i++){
        if (camNames[i] == "Pupil Cam2 ID0"){
            rightEyeCam.init(camNames[i], i);
            rightEyeCam.clickXOffset = 0;
        }
        else if (camNames[i] == "Pupil Cam2 ID1"){
            leftEyeCam.init(camNames[i], i);
            leftEyeCam.clickXOffset = 608;
        }
        else{
            //Do nothing
        }
    }
    frameProc.FWI.fileName = QCoreApplication::arguments()[1].toStdString().append(".csv");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menubar->hide();
    setMouseTracking(true);
    initSystem();


    timer = new QTimer(this);

    //Turn off buttons before doing anything
    ui->RightCalibration->setEnabled(false);
    ui->LeftCalibration->setEnabled(false);
    ui->RunDiagnostic->setEnabled(false);
    ui->RunTherapeutic->setEnabled(false);
    ui->Ten_PD->setEnabled(false);
    ui->Fifteen_PD->setEnabled(false);
    ui->gridLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);

    //Disable sliders

    disableSliders();
    alignCameras();
    ColorOrBW = 0;

}



//Functions to open camera stream and update the display
void MainWindow::startCamera(){
    //If timer is running, restart, else, start
    if(elapsed_timer.isValid()){
        elapsed_timer.restart();
        qDebug() << "Restart etimer";
    }
    else{
        elapsed_timer.start();
        qDebug() << "Start etimer";
        qDebug() << "Open Stream";
        connect(timer, SIGNAL(timeout()), this, SLOT(checkElapsedTime()));
        qDebug() << "Slot Connected";
    }
    timer->start(10);
    qDebug() << "Timer Started";
}

void MainWindow::checkElapsedTime(){
    //If time has elapsed, call reset function, else, get new frames
    if(elapsed_timer.elapsed() > RecordingTimer){
        returntoHomeScreen();
    }
    else{
        updateFrame();
    }
}

void MainWindow::updateFrame(){

    //Print time and calculate seconds for writing data to file later
    qDebug() << static_cast<float>(elapsed_timer.elapsed())/60000;
    float current_time = static_cast<float>(elapsed_timer.elapsed())/1000;

    Mat rightEyeImage, leftEyeImage;
    if (ColorOrBW == 0){
        flip(frameProc.getFrame(rightEyeCam.streamInfo.strmh),rightEyeImage, -1);
        leftEyeImage = frameProc.getFrame(leftEyeCam.streamInfo.strmh);
    }
    else{
        Mat temp;
        flip(frameProc.getFrame(rightEyeCam.streamInfo.strmh),temp, -1);
        rightEyeImage = frameProc.processFrame(temp, rightEyeCam.frameProcInfo, rightEyeCam.boundBox, current_time);
        temp.release();
        leftEyeImage = frameProc.processFrame(frameProc.getFrame(leftEyeCam.streamInfo.strmh), leftEyeCam.frameProcInfo, leftEyeCam.boundBox, current_time);
    }
    ui->RightEyeDisplay->setPixmap(QPixmap::fromImage(QImage((unsigned char*) rightEyeImage.data, rightEyeImage.cols, rightEyeImage.rows, rightEyeImage.step, QImage::Format_RGB888)));
    ui->LeftEyeDisplay->setPixmap(QPixmap::fromImage(QImage((unsigned char*) leftEyeImage.data, leftEyeImage.cols, leftEyeImage.rows, leftEyeImage.step, QImage::Format_RGB888)));
    rightEyeImage.release();
    leftEyeImage.release();
}

void MainWindow::mousePressEvent(QMouseEvent *event){
//    qDebug() << "Pressed";
    QPoint point = event->pos();
    if (point.y() >= 0 && point.y() <= 192){
        if(point.x() >= 608 && point.x() <= 800){
            qDebug() << "\nleft Eye Current Values: ";
            cout << leftEyeCam.boundBox.startX << "," << leftEyeCam.boundBox.startY << "," << leftEyeCam.boundBox.endX << "," << leftEyeCam.boundBox.endY << endl;
            leftEyeCam.updateBoundingBox(point.x(), point.y());
            qDebug() << "left Eye New Values: ";
            cout << leftEyeCam.boundBox.startX << "," << leftEyeCam.boundBox.startY << "," << leftEyeCam.boundBox.endX << "," << leftEyeCam.boundBox.endY << endl;
        }
        else if(point.x() >= 0 && point.x() <= 192){
            qDebug() << "\nRight Eye Current Values: ";
            cout << rightEyeCam.boundBox.startX << "," << rightEyeCam.boundBox.startY << "," << rightEyeCam.boundBox.endX << "," << rightEyeCam.boundBox.endY << endl;
            rightEyeCam.updateBoundingBox(point.x(), point.y());
            qDebug() << "Right Eye New Values: ";
            cout << rightEyeCam.boundBox.startX << "," << rightEyeCam.boundBox.startY << "," << rightEyeCam.boundBox.endX << "," << rightEyeCam.boundBox.endY << endl;
        }
        else{
            qDebug() << "Inside Neither Display";
        }
    }
    else{
        qDebug() << "Inside Neither Display";
    }
}


//Closing things
void MainWindow::returntoHomeScreen(){
    //Stop Timer
    timer->stop();

    //Hide displays, controls & cal buttons
    ui->verticalLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);

    //Show home screen
    ui->gridLayoutWidget->setVisible(true);
}

void MainWindow::closeCameras(){
    if(ColorOrBW == 0){
        ColorOrBW = 1;
    }
    if(DataSavingFlag == 3){
        TestCompleteFlag = 1;
    }

    returntoHomeScreen();
}

void MainWindow::on_CloseDisplay_and_Control_clicked()
{
    closeCameras();
}

//Home Screen
void MainWindow::on_Quit_clicked()
{
    rightEyeCam.close(rightEyeCam.streamInfo.ctx);
    leftEyeCam.close(leftEyeCam.streamInfo.ctx);

    close();
}

void MainWindow::on_TrackingSetUp_clicked()
{
    //Hide home screen and cal buttons; Show displays and controls
    ui->gridLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    //Enable Sliders (only needed to allow redoing tracking after calibration)
    enableSliders();

    //Set slider values
    ui->RightEyeThresholdSlider->setValue(rightEyeCam.frameProcInfo.thresh_val);
    ui->RightEyeRadiusSlider->setValue(rightEyeCam.frameProcInfo.max_radius);
    ui->LeftEyeThresholdSlider->setValue(leftEyeCam.frameProcInfo.thresh_val);
    ui->LeftEyeRadiusSlider->setValue(leftEyeCam.frameProcInfo.max_radius);

    DisplaySelector = 0;
    DataSavingFlag = 0;
    RecordingTimer = SetUpTime;
    startCamera();

    ui->RightCalibration->setEnabled(true);
}

void MainWindow::calibrationSetUp(){
    //Hide Home Screen and Display/Controls
    ui->gridLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget->setVisible(false);

    //Show Calibration amounts
    ui->verticalLayoutWidget_2->setVisible(true);

    //Disable Sliders
    disableSliders();
}

void MainWindow::on_RightCalibration_clicked()
{
    calibrationSetUp();
    frameProc.FWI.step = 1;
    DisplaySelector = 0;
    RecordingTimer = CalibrationTime;

    frameProc.FWI.headerWritten = 0;
}

void MainWindow::on_LeftCalibration_clicked()
{
    calibrationSetUp();

    frameProc.FWI.step = 2;
    DisplaySelector = 0;
    RecordingTimer = CalibrationTime;

    frameProc.FWI.headerWritten = 0;
}

void MainWindow::on_RunDiagnostic_clicked()
{
    //Hide home screen and cal buttons
    ui->gridLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);

    //Show display and controls
    ui->verticalLayoutWidget->setVisible(true);

    //Enable sliders
    enableSliders();

    frameProc.FWI.step = 3;
    DisplaySelector = 0;

    Settime ST;
    ST.setModal(true);
    ST.exec();

    RecordingTimer = Test_Time;
    frameProc.FWI.testTime = to_string(Test_Time_H) + "_" + to_string(Test_Time_M);

    startCamera();
    frameProc.FWI.headerWritten = 0;
}

//Calibration
void MainWindow::on_Five_PD_clicked()
{
    frameProc.FWI.calibrationNumber = 5;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();
    ui->Ten_PD->setEnabled(true);
}

void MainWindow::on_Ten_PD_clicked()
{
    frameProc.FWI.calibrationNumber = 10;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();
    ui->Fifteen_PD->setEnabled(true);
}

void MainWindow::on_Fifteen_PD_clicked()
{
    frameProc.FWI.calibrationNumber = 15;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();

    if (frameProc.FWI.calibrationNumber == 15 && frameProc.FWI.step == 1){
        ui->LeftCalibration->setEnabled(true);
        ui->Ten_PD->setEnabled(false);
        ui->Fifteen_PD->setEnabled(false);
    }
    if (frameProc.FWI.calibrationNumber == 15 && frameProc.FWI.step == 2){
        ui->RunDiagnostic->setEnabled(true);
    }
}

//Sliders
void MainWindow::on_RightEyeThresholdSlider_valueChanged(int RThresh)
{
    ui->RightEyeThresholdDisplay->display(RThresh);
    rightEyeCam.frameProcInfo.thresh_val = RThresh;
}

void MainWindow::on_LeftEyeThresholdSlider_valueChanged(int LThresh)
{
    ui->LeftEyeThresholdDisplay->display(LThresh);
    leftEyeCam.frameProcInfo.thresh_val = LThresh;
}

void MainWindow::on_RightEyeRadiusSlider_valueChanged(int RRad)
{
    ui->RightEyeRadiusDisplay->display(RRad);
    rightEyeCam.frameProcInfo.max_radius = RRad;
}

void MainWindow::on_LeftEyeRadiusSlider_valueChanged(int LRad)
{
    ui->LeftEyeRadiusDisplay->display(LRad);
    leftEyeCam.frameProcInfo.max_radius = LRad;
}

void MainWindow::disableSliders(){
    ui->RightEyeThresholdSlider->setEnabled(false);
    ui->RightEyeRadiusSlider->setEnabled(false);
    ui->LeftEyeThresholdSlider->setEnabled(false);
    ui->LeftEyeRadiusSlider->setEnabled(false);
}

void MainWindow::enableSliders(){
    ui->RightEyeThresholdSlider->setEnabled(true);
    ui->RightEyeRadiusSlider->setEnabled(true);
    ui->LeftEyeThresholdSlider->setEnabled(true);
    ui->LeftEyeRadiusSlider->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}
