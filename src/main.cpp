// #include "mainwindow.h"
// #include "rtc/rtc.hpp"
#include <QApplication>
#include "ip_edit.h"
// #include <webrtc.h>
// #include <functional>
// #include "nozzleCalib.h"
// #include "utils.h"
// #include <QDir>
// #include <memory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    IPEdit ipEdit;
    ipEdit.show();


    // Utils::SYS_DIR = QCoreApplication::applicationDirPath() + "/sys/";
    // Utils::USR_DIR = QCoreApplication::applicationDirPath() + "/usr/";

    // QDir dir;
    // if (!dir.exists(Utils::SYS_DIR)) {
    //     Utils::critical(nullptr, "Error", "多喷嘴偏移配置文件不存在，检查路径: " + Utils::SYS_DIR);
    //     return -1;
    // }

    // if (!dir.exists(Utils::USR_DIR) && !dir.mkpath(Utils::USR_DIR) ) {
    //     qDebug() <<"Can't create dir : " << Utils::USR_DIR << " errno = " << errno;
    //     return -1;
    // }

    // MainWindow* w = MainWindow::getInstance();
    // Webrtc::getInstance()->onPosChangeCallback([=](double x, double y, double z) {
    //     emit w->sigKlippyPosChange(x, y, z);
    // });

    // std::shared_ptr<Decoder> decoder = std::make_shared<Decoder>();
    // Webrtc::getInstance()->setDecoder(decoder);
    // QObject::connect(decoder.get(), &Decoder::signalNewFrame, w, &MainWindow::slotShowVideo);

    // std::shared_ptr<nozzleCalib> calib = std::make_shared<nozzleCalib>();
    // calib->init(Utils::SYS_DIR.toStdString(), Utils::USR_DIR.toStdString());
    // w->setCalib(calib);
    // w->show();

    return a.exec();
}
