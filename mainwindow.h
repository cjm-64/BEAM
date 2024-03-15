#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

private slots:
    //Cam/Stream setup
    void initFrameProc();
    void initCams();

    //Getting frame functions
    void startCamera();
    void checkElapsedTime();
    void updateFrame();

    //Reset Functions
    void returntoHomeScreen();
    void closeCameras();

    //Actions
    void alignCameras();
    void disableSliders();
    void enableSliders();
    void calibrationSetUp();
    void on_Quit_clicked();
    void on_TrackingSetUp_clicked();
    void on_RightCalibration_clicked();
    void on_LeftCalibration_clicked();
    void on_Five_PD_clicked();
    void on_Ten_PD_clicked();
    void on_Fifteen_PD_clicked();
    void on_RunDiagnostic_clicked();
    void on_CloseDisplay_and_Control_clicked();
    void on_RightEyeThresholdSlider_valueChanged(int value);
    void on_LeftEyeThresholdSlider_valueChanged(int value);
    void on_RightEyeRadiusSlider_valueChanged(int value);
    void on_LeftEyeRadiusSlider_valueChanged(int value);
};
#endif // MAINWINDOW_H
