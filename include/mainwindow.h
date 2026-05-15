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

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

typedef enum {
    SHOW_NONE,
    SHOW_VIDEO,
    SHOW_IMG
} SHOW_STATE;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow *getInstance() {
        static MainWindow instance;
        return &instance;
    }

    QVector<QPair<QImage, QVector3D>> getCheckedImgsWithPos();
    void openVideo();
    QImage getCurImage();
    void addImage(const QString &file);

public slots:
    void slotShowVideo(QImage image);

private:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sigKlippyPosChange(double x, double y, double z);

private slots:
    void connectBtn_toggled(bool isChecked);
    void showVideoBtn_toggled(bool isChecked);
    void getImgBtn_clicked();
    void klippyPosChanged(double x, double y, double z);
    void slotMoveBtnClicked();
    void slotXyZeroBtnClicked();
    void slotZZeroBtnClicked();

    void slotAddImgs();
    void showDefaultImg();
    void showCaptureImg(QListWidgetItem *item);
    void slotContextMenuPop(const QPoint &pos);
    void queryVideoDevs();
    void clearActClicked();
    void deleteActClicked();
    void showActClicked();
    void saveActClicked();
    void selectAllClicked();

    void manualCBoxStateChanged(int state);

private:
    void createConnections();
    void connectServer();
    void disConnectServer();
    void menuInit();

    void showImg(QImage img);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::MainWindow *ui;
    QGraphicsPixmapItem item;
    QImage curImg;
    SHOW_STATE state;
    QMenu *actMenu;
};

#endif // MAINWINDOW_H
