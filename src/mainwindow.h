#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <memory>
#include <QTimer>
#include <QListWidgetItem>
#include <QMenu>
#include <memory>
#include <mutex>
#include "RtcPacket.h"
#include "utils.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slotShowVideo(VideoFrame *frame);
    void connectBtn_toggled(bool isChecked);
    void showVideoBtn_toggled(bool isChecked);

private:
    void openVideo();
    void createConnections();
    void connectServer();
    void disConnectServer();
    void addImage(const QString &file);
    void slotAddImgs();
    void queryVideoDevs();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
