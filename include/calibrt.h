#ifndef CALIBRT_H
#define CALIBRT_H

#include <QWidget>
#include <memory>

namespace Ui {
class CalibRT;
}

class CalibRT : public QWidget
{
    Q_OBJECT

public:
    explicit CalibRT(QWidget *parent = nullptr);
    ~CalibRT();

private slots:
    void calculate();

    void transRes();

    void autoCalib();

private:
    void showRes();

    void createConnects();

private:
    const int RT_POSX = 2;
    const int RT_POSY = 333;
    const int RT_POSZ = 30;
    const int RT_NUM = 9;

    Ui::CalibRT *ui;

    QString rtDir;
    QString rtDrawDir;
};

#endif // CALIBRT_H
