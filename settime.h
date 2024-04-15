#ifndef SETTIME_H
#define SETTIME_H

#include <QDialog>

namespace Ui {
class Settime;
}

class Settime : public QDialog
{
    Q_OBJECT

public:
    explicit Settime(QWidget *parent = nullptr);
    ~Settime();

private:
    Ui::Settime *ui;

private slots:

    void on_HoursSlider_valueChanged(int value);
    void on_MinutesSlider_valueChanged(int value);
    void on_ConfirmTime_clicked();
};

#endif // SETTIME_H
