#ifndef SETTIME_H
#define SETTIME_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QShowEvent>
#include <QTimer>
#include <QString>
#include <QDebug>

namespace Ui {
class SetTime;
}

class SetTime : public QDialog
{
    Q_OBJECT

public:
    explicit SetTime(QWidget *parent = nullptr);
    ~SetTime();

public slots:
    void newConnection();

    void writeToSocket();

private slots:
    void on_HourSlider_valueChanged(int value);

    void on_MinuteSlider_valueChanged(int value);

    void on_TimeSet_clicked();

private:
    Ui::SetTime *ui;
    QTcpServer *TimeServer;
    QTcpSocket *TimeSocket;
};

#endif // SETTIME_H
