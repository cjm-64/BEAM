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

    //Sliders
    void on_R_RadiusSlider_valueChanged(int value);
    void on_R_ThresholdSlider_valueChanged(int value);
    void on_L_RadiusSlider_valueChanged(int value);
    void on_L_ThresholdSlider_valueChanged(int value);

    //Actions
    void alignCameras();
    void on_Quit_clicked();
    void on_TrackingSetUp_clicked();
    void on_R_CloseCam_clicked();
    void on_L_CloseCam_clicked();
    void on_RightCal_clicked();
    void on_LeftCal_clicked();
    void on_RunTest_clicked();
    void on_Degree_1_Button_clicked();
};
#endif // MAINWINDOW_H
