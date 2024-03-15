#include "settime.h"
#include "ui_settime.h"
#include "globals.h"

Settime::Settime(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settime)
{
    ui->setupUi(this);
}

Settime::~Settime()
{
    delete ui;
}

void Settime::on_HoursSlider_valueChanged(int H)
{
    ui->HoursDisplay->display(H);
    Test_Time_H = H;
}


void Settime::on_MinutesSlider_valueChanged(int M)
{
    ui->MinutesDisplay->display(M);
    Test_Time_M = M;
}


void Settime::on_ConfirmTime_clicked()
{
    Test_Time = ((Test_Time_H*60*60) + (Test_Time_M*60))*1000;
    close();
}
