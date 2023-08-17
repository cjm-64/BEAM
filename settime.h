#ifndef SETTIME_H
#define SETTIME_H

#include <QDialog>

namespace Ui {
class settime;
}

class settime : public QDialog
{
    Q_OBJECT

public:
    explicit settime(QWidget *parent = nullptr);
    ~settime();

private:
    Ui::settime *ui;



private slots:
    void on_HoursSlider_valueChanged(int value);

    void on_MinutesSlider_valueChanged(int value);

    void on_ConfirmTime_clicked();
};

#endif // SETTIME_H
