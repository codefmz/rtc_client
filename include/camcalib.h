#ifndef CAMCALIB_H
#define CAMCALIB_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <memory>

namespace Ui {
class CamCalib;
}

class CamCalib : public QWidget
{
    Q_OBJECT

public:
    explicit CamCalib(QWidget *parent = nullptr);
    ~CamCalib();

private slots:
    void calculate();

    void showRes();

private:
    void createConnects();

private:
    Ui::CamCalib *ui;
};

#endif // CAMCALIB_H
