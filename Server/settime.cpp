#include "settime.h"
#include "ui_settime.h"
#include "globals.h"

using namespace std;

SetTime::SetTime(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetTime)
{
    ui->setupUi(this);

    TimeServer = new QTcpServer(this);
    connect(TimeServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    //Set up server and check to ensure it worked
    if(!TimeServer->listen(QHostAddress::Any, 1235)){
        qDebug() << "Time server could not start";
    }
    else{
        qDebug() << "Time server started";
    }
}

void SetTime::newConnection()
{
    TimeSocket = TimeServer->nextPendingConnection();
    qDebug() << "Time sock connected";
}

void SetTime::writeToSocket()
{
    qDebug() << "Seg";
    QByteArray TimetoWrite = QByteArray::number(Test_Time);
    qDebug() << "Fault";
    TimeSocket->write(TimetoWrite);
    qDebug() << "In";
    TimeSocket->flush();
    qDebug() << "This";
}

SetTime::~SetTime()
{
    delete ui;
}

void SetTime::on_HourSlider_valueChanged(int Hours)
{
    ui->HourDisplay->display(Hours);
    Test_Time_H = Hours;
}

void SetTime::on_MinuteSlider_valueChanged(int Minutes)
{
    ui->MinuteDisplay->display(Minutes);
    Test_Time_M = Minutes;
}

void SetTime::on_TimeSet_clicked()
{
    Test_Time = ((Test_Time_H*60*60) + (Test_Time_M*60))*1000;
    writeToSocket();
    close();
}
