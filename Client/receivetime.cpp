#include "receivetime.h"
#include "ui_receivetime.h"
#include "globals.h"

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

//Device Communication
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

QByteArray TimePlaceholder;

ReceiveTime::ReceiveTime(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiveTime)
{
    ui->setupUi(this);

    TimeSock = new QTcpSocket(this);

    ClientSetUp();
}

void ReceiveTime::ClientSetUp()
{
    connect(TimeSock, SIGNAL(connected()),this, SLOT(connected()));
    qDebug() << "Slot Connect";
    connect(TimeSock, SIGNAL(readyRead()),this, SLOT(readyRead()));
    qDebug() << "Slot RR";

    qDebug() << "Connecting";

    usleep(1000000);

    TimeSock->connectToHost(IP, 1235);

    qDebug() << "CTH";
    if(TimeSock->waitForConnected(5000)){
        qDebug() << "WFC";
    }
    else{
        qDebug() << "Error: " << TimeSock->errorString();
        close();
    }
    qDebug() << "CSU End";
}

void ReceiveTime::connected(){
    qDebug() << "Connected";
}

void ReceiveTime::readyRead(){
    qDebug() << "RR";

    TimePlaceholder = TimeSock->readAll();
    qDebug() << "Read";
    qDebug() << TimePlaceholder;

    Test_Time = TimePlaceholder.toInt();

    close();
}

ReceiveTime::~ReceiveTime()
{
    delete ui;
}
