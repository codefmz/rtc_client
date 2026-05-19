#ifndef NOZZLE_CALIB_H
#define NOZZLE_CALIB_H

#include <QWidget>
#include <memory>
#include <QButtonGroup>
#include <opencv2/opencv.hpp>

namespace Ui {
class NozzleCalib;
}

class NozzleCalib : public QWidget
{
    Q_OBJECT

public:
    explicit NozzleCalib(QWidget *parent = nullptr);
    ~NozzleCalib();

private:
    Ui::NozzleCalib *ui;
};

#endif // NOZZLE_CALIB_H
