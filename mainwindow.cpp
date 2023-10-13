#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "settime.h"

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

// ID0 = right, ID1 = Left, ID2 = World
struct CamInfo{
    string CamName;
    int CamNum;
    int width;
    int height;
    int fps;
    uint8_t vID;
    uint8_t pID;
    string uid;
};

struct StreamingInfo{
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;
    uvc_stream_handle_t *strmh;
    const uvc_format_desc_t *format_desc;
    uvc_frame_desc_t *frame_desc;
    enum uvc_frame_format frame_format;
};

struct FrameProcessingInfo{
    int thresh_val; //Threshold value for gray to binary conversion
    int max_radius; //Max radius circle to search for in HoughCircles()
};

struct PositionData{
    int X_Pos;
    int Y_Pos;
    int Radius;
};

//Instantiate global data strucutres
CamInfo Cameras[2];
StreamingInfo CamStreams[2];
FrameProcessingInfo FrameProc[2];


//Decompression
tjhandle decompressor = tjInitDecompress();


//Elapsed time to include in data
QElapsedTimer elapsed_timer;

//Other global declarations
Scalar col = Scalar(0, 255, 0); //Color for drawing on frame

//Data Saving
int save_placeholder[4] = {0};
int calibration_number = 1;
string headers[3][1] = {{"Right_Calibration"}, {"Left_Calibration"}, {"Test_Data"}};
int header_num = 0;
ofstream Output_file;

//Initial functions to set up cameras
void getCamInfo(struct CamInfo *ci){
    uvc_context_t *ctx;
    uvc_device_t **device_list;
    uvc_device_t *dev;
    uvc_device_descriptor_t *desc;
    uvc_error_t res;

    res = uvc_init(&ctx,NULL);
    if(res < 0) {
        uvc_perror(res, "uvc_init");
    }
    res = uvc_get_device_list(ctx,&device_list);
    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
    }
    int i = 0;
    int Right_or_Left = 0; //0 = Right; 1 = Left
    while (true) {
        dev = device_list[i];
        if (dev == NULL) {
            break;
        }
        else{
            uvc_get_device_descriptor(dev, &desc);
            printf("Got desc\n");
            if((string)desc->product == "Pupil Cam2 ID0"){
                cout << "Right Eye Camera: " << desc->product << endl;
                Right_or_Left = 0;
            }
            else if((string)desc->product == "Pupil Cam2 ID1"){
                cout << "Left Eye Camera: " << desc->product << endl;
                Right_or_Left = 1;
            }
            else{
                cout << "World Camera: " << desc->product << endl;
                i++;
                uvc_free_device_descriptor(desc);
                continue;
            }
            ci[Right_or_Left].CamName = desc->product;
            ci[Right_or_Left].vID = desc->idVendor;
            ci[Right_or_Left].pID = desc->idProduct;
            ci[Right_or_Left].uid = to_string(uvc_get_device_address(dev))+":"+to_string(uvc_get_bus_number(dev));
            ci[Right_or_Left].CamNum = i;
        }
        uvc_free_device_descriptor(desc);
        i++;
    }
    uvc_exit(ctx);
}

void setUpStreams(struct CamInfo *ci, struct StreamingInfo *si){
    uvc_error_t res;
    uvc_device_t **devicelist;
    uvc_device_t *dev;
    uvc_device_descriptor_t *desc;

    for(int i = 0; i<2;++i){
        res = uvc_init(&si[i].ctx, NULL);
        if (res < 0) {
            uvc_perror(res, "uvc_init");
        }
        else{
            printf("UVC %d open success\n", i);
        }
        res = uvc_find_devices(si[i].ctx, &devicelist, 0, 0, NULL);
        if (res < 0) {
            uvc_perror(res, "uvc_init");
        }
        else{
            for (int j = 0; j < 3; ++j){
                dev = devicelist[j];
                uvc_get_device_descriptor(dev, &desc);
                cout << "   Dev " << j << ": " << dev << " Name: " << desc->product << endl;
            }
        }

        res = uvc_open(devicelist[ci[i].CamNum], &si[i].devh, 1);
        if (res < 0) {
            uvc_perror(res, "uvc_find_device"); /* no devices found */
        }
        else{
            cout << "devh " << i << ": " << si[i].devh << endl;
            cout << "Name " << i << ": " << ci[i].CamName << endl;
        }
        si[i].format_desc = uvc_get_format_descs(si[i].devh);
        si[i].format_desc = si[i].format_desc->next;
        si[i].frame_desc = si[i].format_desc->frame_descs->next;
        si[i].frame_format = UVC_FRAME_FORMAT_MJPEG;
        if(si[i].frame_desc->wWidth != NULL){
            ci[i].width = si[i].frame_desc->wWidth;
            ci[i].height = si[i].frame_desc->wHeight;
            ci[i].fps = 10000000 / si[i].frame_desc->intervals[2];
        }
        printf("\nEye %d format: (%4s) %dx%d %dfps\n", i, si[i].format_desc->fourccFormat, ci[i].width, ci[i].height, ci[i].fps);

        res = uvc_get_stream_ctrl_format_size(si[i].devh, &si[i].ctrl, si[i].frame_format, ci[i].width, ci[i].height, ci[i].fps, 1);
        if (res < 0){
            uvc_perror(res, "start_streaming");
        }
        else{
            printf("Eye %d stream control formatted\n", i);
            uvc_print_stream_ctrl(&si[i].ctrl, stderr);
        }
        res = uvc_stream_open_ctrl(si[i].devh, &si[i].strmh, &si[i].ctrl,1);
        if (res < 0){
            uvc_perror(res, "start_streaming");
        }
        else{
            printf("Eye %d stream opened\n", i);
        }
        res = uvc_stream_start(si[i].strmh, nullptr, nullptr,1.6,0);
        if (res < 0){
            uvc_perror(res, "start_streaming");
        }
        else{
            printf("Eye %d stream started\n", i);
        }
        printf("\n\n\n");
    }
    sleep(1);
}

void MainWindow::initCams(){
    CamInfo CameraInfo[2];
        for(int i = 0; i<2;i++){
            if(i == 0){
                CameraInfo[i].CamName = "Pupil Cam2 ID0";
            }
            else{
                CameraInfo[i].CamName = "Pupil Cam2 ID1";
            }
            CameraInfo[i].width = 192;
            CameraInfo[i].height = 192;
            CameraInfo[i].fps = 120;
        }
        getCamInfo(CameraInfo);
        for (int j = 0; j<2; j++){
            cout << "Cam " << j << ": \n" << CameraInfo[j].CamName << endl;
        }
    setUpStreams(CameraInfo, CamStreams);
}

void MainWindow::initFrameProc(){
    //Set the inital values for the frame proc struct
    for (int i=0; i<2; i++){
        FrameProc[i].thresh_val = 50;
        FrameProc[i].max_radius = 50;
    }
}

void MainWindow::alignCameras(){
    ColorOrBW = 0;
    RecordingTimer = SetUpTime;
    startCamera();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);

    //Initialize cameras and data saving methods
    initCams();
    initFrameProc();

    //Turn off buttons before doing anything
    ui->RightCalibration->setEnabled(false);
    ui->LeftCalibration->setEnabled(false);
    ui->RunDiagnostic->setEnabled(false);
    ui->RunTherapeutic->setEnabled(false);
    ui->Ten_PD->setEnabled(false);
    ui->Fifteen_PD->setEnabled(false);

    //Hide Display, Controls, & Cal Buttons
    ui->verticalLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);

    alignCameras();
    ColorOrBW = 0;

}

//Functions to collect and write data to file
void writeToFile(ofstream &file, PositionData &pd, int eye, float current_time){

    if (eye == 0 && headerwritten == 0){
        if(step == 3){
            file << headers[step-1][0] << "_" << Test_Time_H << "_" << Test_Time_M << ",";
        }
        else{
            file << headers[step-1][0] << "_" << calibration_number << "_pd" << ",";
        }
        file << pd.X_Pos << "," << pd.Y_Pos << ",";
        headerwritten = 1;
    }
    else if (eye == 0 && headerwritten == 1){
        file << " ," << pd.X_Pos << "," << pd.Y_Pos << ",";
    }
    else{
        file << pd.X_Pos << "," << pd.Y_Pos << "," << current_time/1000 << endl;
    }

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
    timer->start(33);
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

    qDebug() << elapsed_timer.elapsed();
    float current_time = elapsed_timer.elapsed();

    uvc_frame_t *frame;
    uvc_error_t res;

    for (int i = 0; i<2;i++){
        //As a side note, the images are greyscaled, however they are treated as color when they are fetched

        Mat image;

        res = uvc_stream_get_frame(CamStreams[i].strmh, &frame, 1 * pow(10,6));
        if(res < 0){
            uvc_perror(res, "Failed to get frame");
            continue;
        }
        else{
//            printf("got frame\n");
        }

        //Allocate buffers for conversions
        int frameW = frame->width;
        int frameH = frame->height;
        long unsigned int frameBytes = frame->data_bytes;

//        printf("Eye %d: frame_format = %d, width = %d, height = %d, length = %lu\n", i, frame->frame_format, frameW, frameH, frameBytes);

        if (frame->frame_format == 7){
            printf("Frame Format: MJPEG\n");
            long unsigned int _jpegSize = frameBytes;
            unsigned char buffer[frameW*frameH*3];
            tjDecompress2(decompressor, (unsigned char *)frame->data, _jpegSize, buffer, frameW, 0, frameH, TJPF_RGB, TJFLAG_FASTDCT);
            Mat placeholder(frameH, frameW, CV_8UC3, buffer);
            placeholder.copyTo(image);
            placeholder.release();
        }
        else if (frame->frame_format == 3){
            printf("Frame Format: Other\n");
            uvc_frame_t *rgb;
            rgb = uvc_allocate_frame(frameW * frameH * 3);
            if (!rgb) {
                printf("unable to allocate bgr frame!\n");
                return;
            }
            uvc_error_t res = uvc_yuyv2rgb(frame, rgb);
            if (res < 0){
                printf("Unable to copy frame to bgr!\n");
            }
            Mat placeholder(rgb->height, rgb->width, CV_8UC3, rgb->data);
            placeholder.copyTo(image);
            placeholder.release();
            uvc_free_frame(rgb);
        }
        else{
            printf("Error, somehow you got to a frame format that doesn't exist\n");
        }

        Mat grayIMG, binaryIMG, bpcIMG; //Create new Mats to to image processing steps
        cvtColor(image, grayIMG, COLOR_BGR2GRAY); //Convert to grayscale
        threshold(grayIMG, binaryIMG, FrameProc[i].thresh_val, thresh_max_val, thresh_type); //Convert to binary based on thresh; controlled by slider
        cvtColor(binaryIMG, bpcIMG, COLOR_GRAY2RGB); // enable color on binary so we can draw on it later

        PositionData pd;
        vector<Vec3f> circles;
        HoughCircles(binaryIMG, circles, HOUGH_GRADIENT, 1, 1000, CED, Cent_D, FrameProc[i].max_radius-2, FrameProc[i].max_radius);
        Vec3i c;
        for( size_t i = 0; i < circles.size(); i++ ){
            c = circles[i];
        }

        pd.X_Pos = c[0];
        pd.Y_Pos = c[1];
        pd.Radius = c[2];
        if(step != 0){
            if (Output_file.is_open()){
                writeToFile(Output_file, pd, i, current_time);
            }
            else{
                Output_file.open("output.csv");
                Output_file << "Header,Right_Eye_X,Right_Eye_Y,Left_Eye_X,Left_Eye_Y,Time_s" << endl;
                writeToFile(Output_file, pd, i, current_time);
            }
        }

        //Draw Circles on Black and White
        circle(bpcIMG, Point(pd.X_Pos, pd.Y_Pos), 1, col,1,LINE_8);
        circle(bpcIMG, Point(pd.X_Pos, pd.Y_Pos), pd.Radius, col,1,LINE_8);

        Mat final_image;
        //Display Image
        //Check for which eye and if grey or Black and White (binary)
        if (i == 0){
            if (ColorOrBW == 0){
                //Right Eye Color frame
                flip(image,final_image, 0);
            }
            else{
                //Right Eye BW frame
                flip(bpcIMG,final_image, 0);
            }
            ui->RightEyeDisplay->setPixmap(QPixmap::fromImage(QImage((unsigned char*) final_image.data, final_image.cols, final_image.rows, final_image.step, QImage::Format_RGB888)));
        }
        else{
            if (ColorOrBW == 0){
                //Right Eye Color frame
                image.copyTo(final_image);
            }
            else{
                //Right Eye BW frame
                bpcIMG.copyTo(final_image);
            }
            ui->LeftEyeDisplay->setPixmap(QPixmap::fromImage(QImage((unsigned char*) final_image.data, final_image.cols, final_image.rows, final_image.step, QImage::Format_RGB888)));
        }

        //Free memory
        image.release();
        grayIMG.release();
        binaryIMG.release();
        bpcIMG.release();
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
    for(int i = 0; i<2; i++){
        uvc_exit(CamStreams[i].ctx);
        cout << "Closed Cam " << i << endl;
    }

    close();
}

void MainWindow::on_TrackingSetUp_clicked()
{
    //Hide home screen and cal buttons; Show displays and controls
    ui->gridLayoutWidget->setVisible(false);
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    //Enable Sliders (only needed to allow redoing tracking after calibration)
    ui->RightEyeThresholdSlider->setEnabled(true);
    ui->RightEyeRadiusSlider->setEnabled(true);
    ui->LeftEyeThresholdSlider->setEnabled(true);
    ui->LeftEyeRadiusSlider->setEnabled(true);

    //Set slider values
    ui->RightEyeThresholdSlider->setValue(FrameProc[0].thresh_val);
    ui->RightEyeRadiusSlider->setValue(FrameProc[0].max_radius);
    ui->LeftEyeThresholdSlider->setValue(FrameProc[1].thresh_val);
    ui->LeftEyeRadiusSlider->setValue(FrameProc[1].max_radius);

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
    ui->RightEyeThresholdSlider->setEnabled(false);
    ui->RightEyeRadiusSlider->setEnabled(false);
    ui->LeftEyeThresholdSlider->setEnabled(false);
    ui->LeftEyeRadiusSlider->setEnabled(false);
}

void MainWindow::on_RightCalibration_clicked()
{
    calibrationSetUp();
    step = 1;
    DisplaySelector = 0;
    RecordingTimer = CalibrationTime;

    headerwritten = 0;
}

void MainWindow::on_LeftCalibration_clicked()
{
    calibrationSetUp();

    step = 2;
    DisplaySelector = 0;
    RecordingTimer = CalibrationTime;

    headerwritten = 0;
}

void MainWindow::on_RunDiagnostic_clicked()
{
    //Hide home screen and cal buttons
    ui->gridLayoutWidget->setEnabled(false);
    ui->verticalLayoutWidget_2->setEnabled(false);

    //Show display and controls
    ui->verticalLayoutWidget->setEnabled(true);

    //Enable sliders
    ui->RightEyeThresholdSlider->setEnabled(true);
    ui->RightEyeRadiusSlider->setEnabled(true);
    ui->LeftEyeThresholdSlider->setEnabled(true);
    ui->LeftEyeRadiusSlider->setEnabled(true);

    step = 3;
    DisplaySelector = 0;

    Settime ST;
    ST.setModal(true);
    ST.exec();

    RecordingTimer = Test_Time;

    startCamera();
    headerwritten = 0;
}

//Calibration
void MainWindow::on_Five_PD_clicked()
{
    calibration_number = 5;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();
    ui->Ten_PD->setEnabled(true);
}

void MainWindow::on_Ten_PD_clicked()
{
    calibration_number = 10;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();
    ui->Fifteen_PD->setEnabled(true);
}

void MainWindow::on_Fifteen_PD_clicked()
{
    calibration_number = 15;

    //Hide cal buttons and show displays
    ui->verticalLayoutWidget_2->setVisible(false);
    ui->verticalLayoutWidget->setVisible(true);

    startCamera();

    if (calibration_number == 15 && step == 1){
        ui->LeftCalibration->setEnabled(false);
        ui->Ten_PD->setEnabled(false);
        ui->Fifteen_PD->setEnabled(false);
    }
    if (calibration_number == 15 && step == 2){
        ui->RunDiagnostic->setEnabled(true);
    }
}

//Sliders
void MainWindow::on_RightEyeThresholdSlider_valueChanged(int RThresh)
{
    ui->RightEyeThresholdDisplay->display(RThresh);
    FrameProc[0].thresh_val = RThresh;
}

void MainWindow::on_LeftEyeThresholdSlider_valueChanged(int LThresh)
{
    ui->LeftEyeThresholdDisplay->display(LThresh);
    FrameProc[1].thresh_val = LThresh;
}

void MainWindow::on_RightEyeRadiusSlider_valueChanged(int RRad)
{
    ui->RightEyeRadiusDisplay->display(RRad);
    FrameProc[0].max_radius = RRad;
}

void MainWindow::on_LeftEyeRadiusSlider_valueChanged(int LRad)
{
    ui->LeftEyeRadiusDisplay->display(LRad);
    FrameProc[1].max_radius = LRad;
}

MainWindow::~MainWindow()
{
    delete ui;
}
