#include "camcalib.h"
#include "ui_camcalib.h"
#include "mainwindow.h"
#include "utils.h"
#include "fileManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector3D>

CamCalib::CamCalib(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CamCalib)
{
    ui->setupUi(this);
    createConnects();
}

CamCalib::~CamCalib()
{
    delete ui;
}

void CamCalib::calculate()
{
    QVector<QPair<QImage, QVector3D>> results = MainWindow::getInstance()->getCheckedImgsWithPos();
    if (results.size() < 3) {
        Utils::warning(this, "Warn", "请开启图像捕获并至少选中3张图片.");
        return;
    }

    for (int i = 0; i < results.size(); ++i) {
        cv::Mat mat = Utils::QImageToMat(results[i].first);
        int ret = calib->cameraCalibrate(mat, i == results.size() - 1, false);
        if (ret != 0) {
            Utils::critical(this, "Error", "计算失败，检测是否添加足够的图片以及图片是否符合要求");
            return;
        }
    }

    showRes();
}

void CamCalib::showRes()
{
    QString resFilePath = Utils::SYS_DIR + NOZZLE_CAMPARAMS_FILE;
    QFile file(resFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Utils::warning(this, "Warn", resFilePath + " 文件不存在.");
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString content = in.readAll();
    file.close();
    ui->resEdit->setPlainText(content);
}

void CamCalib::createConnects()
{
    connect(ui->calcuBtn, SIGNAL(clicked()), SLOT(calculate()));
}

