#ifndef RECEIVETIME_H
#define RECEIVETIME_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

namespace Ui {
class ReceiveTime;
}

class ReceiveTime : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiveTime(QWidget *parent = nullptr);
    ~ReceiveTime();

public slots:
    void connected();
    void readyRead();

private slots:
    void ClientSetUp();

private:
    Ui::ReceiveTime *ui;
    QTcpSocket * TimeSock;
};

#endif // RECEIVETIME_H
