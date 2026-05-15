#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webrtc.h"
#include "utils.h"
#include <QSignalBlocker>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QtConcurrent>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), state(SHOW_NONE)
{
    ui->setupUi(this);
    // ui->ipEdit->setText("192.168.136.128");
    // ui->ipEdit->setText("172.18.208.203");
    ui->ipEdit->setText("172.18.209.63");
    // ui->ipEdit->setText("172.18.211.40");
    // ui->ipEdit->setText("172.18.102.192");

    ui->graphicsView->setScene(new QGraphicsScene);
    ui->graphicsView->scene()->addItem(&item);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    createConnections();
    menuInit();
    this->showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QVector<QPair<QImage, QVector3D>> MainWindow::getCheckedImgsWithPos()
{
    QVector<QPair<QImage, QVector3D>> results;
    for (int i =0; i < ui->imgList->count(); ++i) {
        auto item = ui->imgList->item(i);
        if (item->checkState() == Qt::Checked) {
            QImage img = item->data(ItemImg).value<QImage>();
            QVector3D pos = item->data(ItemPos).value<QVector3D>();
            results.push_back(qMakePair(img, pos));
        }
    }

    return results;
}

void MainWindow::setCalib(std::shared_ptr<nozzleCalib> &calib)
{
    ui->camCalib->setCalib(calib);
    ui->calibRT->setCalib(calib);
    ui->nozzleXY->setCalib(calib);
}

void MainWindow::openVideo()
{
    if (!ui->showVideoBtn->isChecked()) {
        ui->showVideoBtn->setChecked(true);
    }
}

QImage MainWindow::getCurImage()
{
    return curImg;
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
        state = SHOW_VIDEO;
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

        state = SHOW_NONE;
        Webrtc::getInstance()->stopShowVideo();
        ui->showVideoBtn->setText("显示视频");
        showDefaultImg();
    }
}

void MainWindow::getImgBtn_clicked()
{
    if (state != SHOW_VIDEO) {
        Utils::warning(this, "Warn", "取图之前需要开启显示视频！");
        return;
    }

    if (curImg.isNull()) {
        Utils::warning(this, "Warn", "取图失败，图像为空！");
        return;
    }

    float x = ui->xSpin->value();
    float y = ui->ySpin->value();
    float z = ui->zSpin->value();
    QString itemName = "img_" + QString::number(x, 'f', 2) + "_" + QString::number(y, 'f', 2)
        + "_" + QString::number(z, 'f', 2) + ".jpg";
    QListWidgetItem *item = new QListWidgetItem(itemName);
    item->setData(ItemImg, curImg);

    QVector3D qpos(x, y, z);
    item->setData(ItemPos, qpos);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    ui->imgList->addItem(item);
}

void MainWindow::slotShowVideo(QImage image)
{
    if (state != SHOW_VIDEO) {
        return;
    }

    curImg = image;
    if (curImg.isNull()) {
        return;
    }

    item.setPixmap(QPixmap::fromImage(image));
    ui->graphicsView->fitInView(&item);
    ui->graphicsView->update();
}

void MainWindow::klippyPosChanged(double x, double y, double z)
{
    ui->xSpin->setValue(x);
    ui->ySpin->setValue(y);
    ui->zSpin->setValue(z);
}

void MainWindow::slotMoveBtnClicked()
{
    double x = ui->xSpin->value();
    double y = ui->ySpin->value();
    double z = ui->zSpin->value();

    RTC_Pos_Param param = { 0 };
    param.x = x;
    param.y = y;
    param.z = z;

    int ret = Webrtc::getInstance()->sendMessage(CMD_KLIPPY_MOVE, 3, (uint8_t *)&param, sizeof(RTC_Pos_Param));
    if (ret !=  0) {
        Utils::warning(this, "Warn", "Move failed!");
        return;
    }
}

void MainWindow::slotXyZeroBtnClicked()
{
    int ret = Webrtc::getInstance()->sendMessage(CMD_KLIPPY_XY, 30);
    if (ret != 0) {
        Utils::warning(this, "Warn", "Reset to xy fail!");
        return;
    }
}

void MainWindow::slotZZeroBtnClicked()
{
    int ret = Webrtc::getInstance()->sendMessage(CMD_KLIPPY_Z, 120);
    if (ret != 0) {
        Utils::warning(this, "Warn", "Reset to z fail!");
        return;
    }
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
    connect(ui->getImgBtn, SIGNAL(clicked()), this, SLOT(getImgBtn_clicked()));
    connect(ui->addImgBtn, SIGNAL(clicked()), this, SLOT(slotAddImgs()));
    connect(this, SIGNAL(sigKlippyPosChange(double,double,double)), SLOT(klippyPosChanged(double,double,double)));
    connect(ui->moveBtn, SIGNAL(clicked()), SLOT(slotMoveBtnClicked()));
    connect(ui->xyZeroBtn, SIGNAL(clicked()), SLOT(slotXyZeroBtnClicked()));
    connect(ui->zZeroBtn, SIGNAL(clicked()), SLOT(slotZZeroBtnClicked()));
    connect(ui->imgList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(showCaptureImg(QListWidgetItem *)));
    connect(ui->imgList, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotContextMenuPop(const QPoint &)));
    connect(ui->clearAct, SIGNAL(triggered()), SLOT(clearActClicked()));
    connect(ui->deleteAct, SIGNAL(triggered()), SLOT(deleteActClicked()));
    connect(ui->showAct, SIGNAL(triggered()), SLOT(showActClicked()));
    connect(ui->saveAct, SIGNAL(triggered()), SLOT(saveActClicked()));
    connect(ui->nozzleXY, SIGNAL(signalManualCBoxStateChanged(int)), SLOT(manualCBoxStateChanged(int)));
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

void MainWindow::menuInit()
{
    actMenu = new QMenu(this);
    actMenu->addAction(ui->clearAct);
    actMenu->addAction(ui->deleteAct);
    actMenu->addAction(ui->showAct);
    actMenu->addAction(ui->saveAct);
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

void MainWindow::showImg(QImage img)
{
    item.setPixmap(QPixmap::fromImage(img));
    ui->graphicsView->fitInView(&item);
    ui->graphicsView->update();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (state == SHOW_IMG && obj == ui->graphicsView->scene() && event->type() == QEvent::GraphicsSceneMouseDoubleClick) {
        auto *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        QPointF scenePos = mouseEvent->scenePos();
        QGraphicsItem *item = ui->graphicsView->scene()->itemAt(scenePos, QTransform());
        if (auto pixItem = dynamic_cast<QGraphicsPixmapItem*>(item)) {
            QPointF posInItem = pixItem->mapFromScene(scenePos);
            QImage img = pixItem->pixmap().toImage();

            float x = posInItem.x();
            float y = posInItem.y();
            cv::Point3f pos;
            if (x >= 0 && y >= 0 && x < img.width() && y < img.height()) {
                int ret = calib->pixelToWorld({x, y}, pos, 0);
                if (ret == 0) {
                    QString text = "pixelPos(" + QString::number(x, 'f', 2) + ", " + QString::number(y, 'f', 2) +
                        ") -> worldPos(" + QString::number(pos.x, 'f', 2) + ", " + QString::number(pos.y, 'f', 2) +
                        ", " + QString::number(pos.z, 'f', 2) + ")";
                    Utils::information(this, "info", text);
                } else {
                    Utils::warning(this, "Warn", "计算失败");
                }
            }
        }
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showDefaultImg()
{
    qDebug() << " showDefaultImg ";
    item.setPixmap(QPixmap());
    ui->graphicsView->fitInView(&item);
    ui->graphicsView->update();
}

void MainWindow::showCaptureImg(QListWidgetItem *item)
{
    if (ui->showVideoBtn->isChecked()) {
        ui->showVideoBtn->setChecked(false);
    }

    state = SHOW_IMG;
    QImage img = item->data(ItemImg).value<QImage>();
    showImg(img);
}

void MainWindow::slotContextMenuPop(const QPoint &pos)
{
    QPoint globalPos = ui->imgList->mapToGlobal(pos);
    actMenu->exec(globalPos);
}

void MainWindow::clearActClicked()
{
    ui->imgList->clear();
}

void MainWindow::deleteActClicked()
{
    int row = ui->imgList->currentRow();
    if (row == -1) {
        return;
    }
    QListWidgetItem *item = ui->imgList->takeItem(row);
    delete item;
}

void MainWindow::showActClicked()
{
    ui->showVideoBtn->setChecked(false);
    auto item = ui->imgList->currentItem();
    if (item == nullptr) {
        return;
    }

    state = SHOW_IMG;
    QImage img = item->data(ItemImg).value<QImage>();
    showImg(img);
}

void MainWindow::saveActClicked()
{
    auto item = ui->imgList->currentItem();
    if (item == nullptr) {
        return;
    }

    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择保存图片的文件夹"), QDir::homePath());
    if (dirPath.isEmpty()) {
        return;
    }

    QString itemName = item->text();
    QString filePath = dirPath + "/" + itemName + ".jpg";
    QImage img = item->data(Qt::UserRole).value<QImage>();
    img.save(filePath, "JPG");
}

void MainWindow::selectAllClicked()
{

}

void MainWindow::manualCBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->graphicsView->scene()->installEventFilter(this);
    } else if (state == Qt::Unchecked) {
        ui->graphicsView->scene()->removeEventFilter(this);
    }
}

