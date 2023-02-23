#include "mainwindow.h"
#include "ui_mainwindow.h"

// Computer Vision
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

//Image Processing Delcaration
Mat src, gray_of_src, binary_of_gray, binary_plus_color, gray_plus_color, fail_img, imgroi, img;
Scalar col = Scalar(0, 255, 0); // green

//Open Video Stream
VideoCapture cap(0);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Timer = new QTimer(this);
}

void MainWindow::openCamera(){
    if(!cap.isOpened())  // Check if we succeeded
    {
        qDebug() << "camera is not open";
    }
    else
    {
        qDebug() << "camera is open";
        connect(Timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
        qDebug() << "Slot Connected";
        Timer->start(20);
        qDebug() << "Timer Started";
    }
}

void MainWindow::updateFrame(){
    qDebug() << "uF";
    cap >> src;
    qDebug() << "cap2src";
    ui->Video->setPixmap(QPixmap::fromImage(QImage((unsigned char*) src.data, src.cols, src.rows, src.step, QImage::Format_RGB888)));
    qDebug() << "Done";
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_StartVideo_clicked()
{
    openCamera();
}

