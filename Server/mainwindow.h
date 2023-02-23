#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QShowEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
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

public slots:
    void newConnection();

    void writeToSocket();

private slots:

    void setFileName();

    void openCamera();

    void updateFrame();

    void returntoHomeScreen();

    void on_TrackingSetUp_clicked();

    void on_CloseControls_clicked();

    void on_CalibrateRight_clicked();

    void on_CalibrateLeft_clicked();

    void on_Diagnostic_clicked();

    void on_Quit_clicked();

    void on_ThresholdSlider_valueChanged(int value);

    void on_CircleSizeSlider_valueChanged(int value);

    void on_BoxXSlider_valueChanged(int value);

    void on_BoxYSlider_valueChanged(int value);

    void on_BoxHSlider_valueChanged(int value);

    void on_BoxWSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QTcpServer *Server;
    QTcpSocket *Socket;
    QTimer *Timer;
};
#endif // MAINWINDOW_H
