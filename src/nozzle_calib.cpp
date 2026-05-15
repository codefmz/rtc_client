#include "nozzle_calib.h"
#include "ui_nozzlecalib.h"
#include "mainwindow.h"
#include "utils.h"
#include "webrtc.h"
#include "fileManager.h"
#include <QVector3D>
#include <QDir>

constexpr int POSX = -2;
constexpr int POSY = 313;
constexpr int POSZ = 30;
constexpr int NOZZLE_NUM = 6;

NozzleCalib::NozzleCalib(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NozzleCalib)
{
    ui->setupUi(this);

    QDir dir;
    offsetDir = Utils::USR_DIR + AI_SAVE_IMG_PATH + NOZZLE_OFFSET_PATH;
    if (!dir.exists(offsetDir) && !dir.mkpath(offsetDir)) {
        qDebug() <<"make path faile, offsetDir = " << offsetDir;
    }

    offsetDrawDir = offsetDir + "draw/";
    if (!dir.exists(offsetDrawDir) && !dir.mkpath(offsetDrawDir)) {
        qDebug() <<"make path faile, offsetDrawDir = " << offsetDrawDir;
    }

    mGroup = new QButtonGroup(this);
    mGroup->addButton(ui->nozzleZero, 0);
    mGroup->addButton(ui->nozzleOne, 1);
    mGroup->addButton(ui->nozzleTwo, 2);
    mGroup->addButton(ui->nozzleThree, 3);
    mGroup->addButton(ui->nozzleFour, 4);
    mGroup->addButton(ui->nozzleFive, 5);

    connect(ui->manualCBox, SIGNAL(stateChanged(int)), this, SLOT(onManualCBoxStateChanged(int)));
    connect(ui->calcuBtn, SIGNAL(clicked()), this, SLOT(calculate()));
    connect(ui->autoCalibBtn, SIGNAL(clicked()), this, SLOT(autoCalib()));
    connect(ui->nozzleTypeBtn, SIGNAL(clicked()), this, SLOT(nozzleTypeCalib()));
    connect(mGroup, &QButtonGroup::idClicked, this, &NozzleCalib::nozzleBtnClicked);
}

NozzleCalib::~NozzleCalib()
{
    delete ui;
}

void NozzleCalib::onManualCBoxStateChanged(int state)
{
    emit signalManualCBoxStateChanged(state);
}

void NozzleCalib::calculate()
{
    QVector<QPair<QImage, QVector3D>> results = MainWindow::getInstance()->getCheckedImgsWithPos();
    if (results.empty()) {
        Utils::warning(this, "Error", "先选择需要检测的图片!");
        return;
    }

    std::vector<cv::Point2f> pixelPts;
    int ret = -1;
    for (int i = 0; i < results.size(); ++i) {
        cv::Mat mat = Utils::QImageToMat(results[i].first);
        cv2d::Circle circle;
        cv::Rect rect;
        ret = calib->detectNozzle(mat, circle, rect);
        if (ret != 0) {
            Utils::critical(this, "Error", "检测图片圆失败");
            return;
        }

        pixelPts.emplace_back(circle.center);
    }

    std::vector<cv::Point3f> worldPts;
    ret = calib->pixelToWorld(pixelPts, worldPts);
    if (ret != 0) {
        Utils::critical(this, "Error", "计算失败");
        return;
    }

    showRes(pixelPts, worldPts);
}

void NozzleCalib::showRes(const std::vector<cv::Point2f> &pixelPts, const std::vector<cv::Point3f> &worldPts)
{
    QString text;
    for (int i = 0; i < worldPts.size(); ++i) {
        const cv::Point2f &pp = pixelPts[i];
        const cv::Point3f &wp = worldPts[i];
        text += QString("Point %1: pixelPoint(%2, %3, %4) - worldPoint(%5, %6, %7)\n").arg(i)
            .arg(pp.x).arg(pp.y)
            .arg(wp.x).arg(wp.y).arg(wp.z);
    }

    ui->resEdit->setPlainText(text);
}

void NozzleCalib::autoCalib()
{
    // MainWindow::getInstance()->openVideo();

    // std::vector<cv::Point2f> pixelPts;
    // RTC_CALIB_OFFSET_PARAM param = { 0 };
    // int ret;
    // param.posX = POSX;
    // param.posY = POSY;
    // param.posZ = POSZ;

    // for (int i = 0; i < NOZZLE_NUM; ++i) {
    //     param.nozzleT = i;
    //     ret = Webrtc::getInstance()->sendMessage(CMD_CALIB_OFFSET_PRE, 240, (uint8_t *)&param, sizeof(RTC_CALIB_OFFSET_PARAM));
    //     if (ret != 0) {
    //         Utils::critical(this, "Error", "偏移校准前处理失败!");
    //         return;
    //     }

    //     QImage img = MainWindow::getInstance()->getCurImage();
    //     cv::Mat mat = Utils::QImageToMat(img);
    //     cv2d::Circle circle;
    //     cv::Rect rect;
    //     std::vector<cv2d::Circle> cirList;

    //     ret = calib->detectNozzle(mat, cirList, circle, rect);
    //     if (ret != 0) {
    //         Utils::critical(this, "Error", "检测图片圆失败");
    //         return;
    //     }

    //     QString drawPath = offsetDrawDir + "offset_draw_" + QString::number(param.posX) + "_" +
    //         QString::number(param.posY) + "_" + QString::number(param.posZ) + ".jpg";
    //     Utils::drawResult(mat, drawPath.toStdString(), cirList, circle, rect);
    //     MainWindow::getInstance()->addImage(drawPath);

    //     pixelPts.emplace_back(circle.center);
    //     ret = Webrtc::getInstance()->sendMessage(CMD_CALIB_OFFSET_POST, 60,
    //         (uint8_t *)&param, sizeof(RTC_CALIB_OFFSET_PARAM));
    //     if (ret != 0) {
    //         Utils::critical(this, "Error", "偏移校准前处理失败!");
    //         return;
    //     }
    // }

    // std::vector<cv::Point3f> worldPts;
    // ret = calib->pixelToWorld(pixelPts, worldPts);
    // if (ret != 0) {
    //     Utils::critical(this, "Error", "计算失败");
    //     return;
    // }

    // showRes(pixelPts, worldPts);
}

void NozzleCalib::nozzleTypeCalib()
{
    QVector<QPair<QImage, QVector3D>> results = MainWindow::getInstance()->getCheckedImgsWithPos();
    if (results.empty()) {
        Utils::warning(this, "Error", "先选择需要检测的图片!");
        return;
    }

    ui->resEdit->clear();
    std::vector<std::string> types;
    for (int i = 0; i < results.size(); ++i) {
        cv::Mat mat = Utils::QImageToMat(results[i].first);
        std::string type = calib->getNozzleType(mat);
        if (type.empty()) {
            Utils::warning(this, "Warn", "Get nozzle type failed.");
            type = "检测失败";
        }
        types.push_back(type);
        ui->resEdit->append(QString::fromStdString(type) +"\n");
    }
}

void NozzleCalib::nozzleBtnClicked(int id)
{
    MainWindow::getInstance()->openVideo();

    std::vector<cv::Point2f> pixelPts;
    RTC_CALIB_OFFSET_PARAM param = { 0 };
    int ret;
    param.posX = POSX;
    param.posY = POSY;
    param.posZ = POSZ;

    param.nozzleT = id;
    ret = Webrtc::getInstance()->sendMessage(CMD_CALIB_OFFSET_PRE, 240, (uint8_t *)&param, sizeof(RTC_CALIB_OFFSET_PARAM));
    if (ret != 0) {
        Utils::critical(this, "Error", "偏移校准前处理失败!");
        return;
    }

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

    QString drawPath = offsetDrawDir + "offset_draw_" + QString::number(param.posX) + "_" +
        QString::number(param.posY) + "_" + QString::number(param.posZ) + ".jpg";
    Utils::drawResult(mat, drawPath.toStdString(), cirList, circle, rect);
    MainWindow::getInstance()->addImage(drawPath);

    pixelPts.emplace_back(circle.center);
    ret = Webrtc::getInstance()->sendMessage(CMD_CALIB_OFFSET_POST, 60,
        (uint8_t *)&param, sizeof(RTC_CALIB_OFFSET_PARAM));
    if (ret != 0) {
        Utils::critical(this, "Error", "偏移校准前处理失败!");
        return;
    }

    std::vector<cv::Point3f> worldPts;
    ret = calib->pixelToWorld(pixelPts, worldPts);
    if (ret != 0) {
        Utils::critical(this, "Error", "计算失败");
        return;
    }

    showRes(pixelPts, worldPts);
}
