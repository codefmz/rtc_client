#ifndef NOZZLE_CALIB_H
#define NOZZLE_CALIB_H

#include <QWidget>
#include <memory>
#include <QButtonGroup>

namespace Ui {
class NozzleCalib;
}

class NozzleCalib : public QWidget
{
    Q_OBJECT

public:
    explicit NozzleCalib(QWidget *parent = nullptr);
    ~NozzleCalib();
signals:
    void signalManualCBoxStateChanged(int state);

private slots:
    void onManualCBoxStateChanged(int state);
    void calculate();
    void autoCalib();
    void nozzleTypeCalib();
    void nozzleBtnClicked(int id);

private:
    void showRes(const std::vector<cv::Point2f> &pixelPts, const std::vector<cv::Point3f> &worldPts);

private:
    Ui::NozzleCalib *ui;
    QString offsetDir;
    QString offsetDrawDir;
    QButtonGroup* mGroup;
};

#endif // NOZZLE_CALIB_H
