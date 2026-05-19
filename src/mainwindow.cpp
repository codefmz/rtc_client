#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webrtc.h"
#include "utils.h"
#include <QSignalBlocker>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->ipEdit->setText("172.18.209.63");


    createConnections();
    this->showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openVideo()
{
    if (!ui->showVideoBtn->isChecked()) {
        ui->showVideoBtn->setChecked(true);
    }
}

void MainWindow::addImage(const QString &file)
{
    QImage img(file);
    QString fileName = QFileInfo(file).fileName();
    QListWidgetItem *item = new QListWidgetItem;
    QStringList parts = fileName.split('_');
    QString itemName = fileName;
    if (parts.size() > 3) {
        bool isOkX = false, isOkY = false, isOkZ = false;
        float x = parts[1].toFloat(&isOkX);
        float y = parts[2].toFloat(&isOkY);
        float z = parts[3].toFloat(&isOkZ);
        if (isOkX && isOkY && isOkZ) {
            itemName = "img_" + QString::number(x, 'f', 2) + "_" + QString::number(y, 'f', 2)
                + "_" + QString::number(z, 'f', 2) + ".jpg";
            QVector3D qpos(x, y, z);
            item->setData(ItemPos, qpos);
        }
    }
    item->setData(ItemImg, img);
    item->setText(itemName);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    ui->imgList->addItem(item);
}

void MainWindow::connectBtn_toggled(bool isChecked)
{
    if (isChecked) {
        connectServer();
    } else {
        disConnectServer();
    }
}

void MainWindow::showVideoBtn_toggled(bool isChecked)
{
    if (isChecked) {
        Webrtc::getInstance()->startShowVideo();
        QString dev = ui->devComBox->currentText();
        int ret = Webrtc::getInstance()->sendMessage(CMD_SHOW_VIDEO, 3,  (uint8_t *)dev.toStdString().data(), dev.length());
        if (ret != 0) {
            qDebug() << "showVideoBtn_toggled ret = " << ret;
            Utils::warning(this, "Warn", "Open dev " + dev + "failed.");
            QSignalBlocker blocker(ui->showVideoBtn);
            ui->showVideoBtn->setChecked(false);
            return;
        }
        ui->showVideoBtn->setText("停止显示");
    } else {
        QString dev = ui->devComBox->currentText();
        int ret = Webrtc::getInstance()->sendMessage(CMD_STOP_SHOW_VIDEO, 3, (uint8_t *)dev.toStdString().data(), dev.length());
        if (ret != 0) {
            Utils::warning(this, "Warn", "Close dev " + dev + "failed.");
            QSignalBlocker blocker(ui->showVideoBtn);
            ui->showVideoBtn->setChecked(false);
            return;
        }

        Webrtc::getInstance()->stopShowVideo();
        ui->showVideoBtn->setText("显示视频");
    }
}

void MainWindow::slotShowVideo(VideoFrame *frame)
{
    ui->playerWgt->inputOneFrame(frame->data, frame->width, frame->height);
    delete frame;
}

void MainWindow::slotAddImgs()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
        "选择多个 JPG 图片",
        "",
        "Images (*.jpg *.jpeg)"
    );

    if (files.isEmpty()) {
        return;
    }

    for (const QString &file : files) {
        addImage(file);
    }
}

void MainWindow::createConnections()
{
    connect(ui->connectBtn, SIGNAL(toggled(bool)), this, SLOT(connectBtn_toggled(bool)));
    connect(ui->acquireDevBtn, SIGNAL(clicked()), this, SLOT(queryVideoDevs()));
    connect(ui->showVideoBtn, SIGNAL(toggled(bool)), this, SLOT(showVideoBtn_toggled(bool)));
}

void MainWindow::connectServer()
{
    QString ip = ui->ipEdit->text();
    if (ip.isEmpty()) {
        Utils::warning(this, "Warn", "连接 " + ip + " 失败！");
        QSignalBlocker blocker(ui->connectBtn);
        ui->connectBtn->setChecked(false);
        return;
    }

    ip.append(":48080");
    qDebug() <<"connected ip : " << ip;
    int ret = Webrtc::getInstance()->connectServer(ip.toStdString());
    if (ret < 0) {
        Utils::warning(this, "Warn", "连接 " + ip + " 失败！");
        ui->showVideoBtn->setChecked(false);
        QSignalBlocker blocker(ui->connectBtn);
        ui->connectBtn->setChecked(false);
        return;
    }

    Utils::information(this, "Info", "连接成功！");
    ui->connectBtn->setText("断开");
}

void MainWindow::disConnectServer()
{
    ui->showVideoBtn->setChecked(false);
    ui->connectBtn->setText("连接");

    int ret = Webrtc::getInstance()->sendMessage(CMD_DISCONNECT);
    if (ret != 0) {
        Utils::warning(this, "Warn", "Disconnect failed.");
        QSignalBlocker blocker(ui->connectBtn);
        ui->connectBtn->setChecked(true);
        return;
    }

    Webrtc::getInstance()->disConnectServer();
}


void MainWindow::queryVideoDevs()
{
    uint8_t *output = nullptr;
    int outLen = 0;
    int ret = Webrtc::getInstance()->sendMessage(CMD_SHOW_DEVS, 3, nullptr, 0, &output, &outLen);
    if (ret != 0) {
        qDebug() <<" ret = " << ret;
        Utils::warning(this, "Warn", "Get Devs failed.");
        return;
    }

    ui->devComBox->clear();
    QString devStr = QString::fromLocal8Bit((char *)output, outLen);
    auto devs = devStr.split("=");
    for (auto & dev: devs) {
        ui->devComBox->addItem(dev);
    }
}
