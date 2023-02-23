#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QShowEvent>
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

public slots:
    void connected();
    void readyRead();

private slots:

    void ClientSetUp();

    void Distro();

    void UpdateFrame();

    void OpenCamera();

    void ReturntoHomeScreen();

    void on_ThresholdSlider_valueChanged(int value);

    void on_CircleSizeSlider_valueChanged(int value);

    void on_BoxXSlider_valueChanged(int value);

    void on_BoxYSlider_valueChanged(int value);

    void on_BoxHSlider_valueChanged(int value);

    void on_BoxWSlider_valueChanged(int value);

    void on_CloseControlPanel_clicked();

protected:
    void showEvent(QShowEvent *ev);

private:
    Ui::MainWindow *ui;

    QTcpSocket * socket;

    QTimer *timer;
};
#endif // MAINWINDOW_H
