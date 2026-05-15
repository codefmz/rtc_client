#include "calibrt.h"
#include "ui_calibrt.h"
#include "mainwindow.h"
#include "utils.h"
#include "fileManager.h"
#include <QVector3D>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include "utils.h"
#include "webrtc.h"

CalibRT::CalibRT(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalibRT)
{
    ui->setupUi(this);
    createConnects();

    QDir dir;
    rtDir = Utils::USR_DIR + AI_SAVE_IMG_PATH + NOZZLE_CALIBRT_PATH;
    if (!dir.exists(rtDir) && !dir.mkpath(rtDir)) {
        qDebug() <<"make path faile, rtDir = " << rtDir;
    }

    rtDrawDir = rtDir + "draw/";
    if (!dir.exists(rtDrawDir) && !dir.mkpath(rtDrawDir)) {
        qDebug() <<"make path faile, rtDrawDir = " << rtDrawDir;
    }
}

CalibRT::~CalibRT()
{
    delete ui;
}

void CalibRT::calculate()
{
    QVector<QPair<QImage, QVector3D>> results = MainWindow::getInstance()->getCheckedImgsWithPos();
    if (results.size() < 9) {
        Utils::warning(this, "Warn", "请开启图像捕获并至少选中3张图片.");
        return;
    }

    std::vector<cv::Point2f> imgPts;
    std::vector<cv::Point3f> objPts;
    int ret = -1;
    for (int i = 0; i < results.size(); ++i) {
        cv::Mat mat = Utils::QImageToMat(results[i].first);
        cv2d::Circle circle;
        cv::Rect rect;
        std::vector<cv2d::Circle> cirList;

        auto qPos = &(results[i].second);
        float x = qPos->x();
        float y = qPos->y();
        float z = qPos->z();

        ret = calib->detectNozzle(mat, cirList, circle, rect);
        if (ret != 0) {
            Utils::critical(this, "Error", "检测图片圆失败");
            return;
        }


        QString drawPath = rtDrawDir + "draw_" + QString::number(x) + "_" +
            QString::number(y) + "_" + QString::number(z) + ".jpg";
        Utils::drawResult(mat, drawPath.toStdString(), cirList, circle, rect);
        MainWindow::getInstance()->addImage(drawPath);

        imgPts.emplace_back(circle.center);
        objPts.emplace_back(x, y, z);
    }

    ret = calib->camToPrintRT(imgPts, objPts);
    if (ret != 0) {
        Utils::critical(this, "Error", "计算失败");
        return;
    }

    showRes();
}


void CalibRT::showRes()
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

void CalibRT::createConnects()
{
    connect(ui->calcuBtn, SIGNAL(clicked()), this, SLOT(calculate()));
    connect(ui->transBtn, SIGNAL(clicked()), this, SLOT(transRes()));
    connect(ui->autocalibBtn, &QPushButton::clicked, this, &CalibRT::autoCalib);
}

void CalibRT::transRes()
{

}

void CalibRT::autoCalib()
{
    MainWindow::getInstance()->openVideo();

    std::vector<cv::Point3f> objPts;
    std::vector<cv::Point2f> imgPts;
    RTC_CALIB_RT_PARAM param = { 0 };
    int ret;
    param.posZ = RT_POSZ;

    for (int i = 0; i < RT_NUM; ++i) {
        param.cnt = i;
        param.posX = -1 + i / 3 + RT_POSX;
        param.posY = -1 + i / 3 + RT_POSY;

        ret = Webrtc::getInstance()->sendMessage(CMD_CALIB_RT, 240, (uint8_t *)&param, sizeof(RTC_CALIB_RT_PARAM));
        if (ret != 0) {
            Utils::critical(this, "Error", "标定处理失败!");
            return;
        }

        qDebug() << "cnt = " << param.cnt << " param.posX = " << param.posX << " param.posY = " << param.posY;

        QImage img = MainWindow::getInstance()->getCurImage();
        cv::Mat mat = Utils::QImageToMat(img);
        cv2d::Circle circle;
        cv::Rect rect;
        std::vector<cv2d::Circle> cirList;

        ret = calib->detectNozzle(mat, cirList, circle, rect);
        if (ret != 0) {
            Utils::critical(this, "Error", "检测图片圆失败");
            return;
        }

        QString drawPath = rtDrawDir + "draw_" + QString::number(param.posX) + "_" +
            QString::number(param.posY) + "_" + QString::number(param.posZ) + ".jpg";
        Utils::drawResult(mat, drawPath.toStdString(), cirList, circle, rect);
        MainWindow::getInstance()->addImage(drawPath);

        imgPts.emplace_back(circle.center);
    }

    std::vector<cv::Point3f> worldPts;
    ret = calib->camToPrintRT(imgPts, objPts, false);
    if (ret != 0) {
        Utils::critical(this, "Error", "计算失败");
        return;
    }

    showRes();
}

