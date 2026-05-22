#include "mainwindow.h"
#include <QApplication>
#include <webrtc.h>
#include <functional>
#include "utils.h"
#include <QDir>
#include <memory>

#include "plog/Appenders/ColorConsoleAppender.h"
#include "plog/Appenders/RollingFileAppender.h"
#include "plog/Formatters/TxtFormatter.h"
#include "plog/Logger.h"
#include "plog/Init.h"
#include "plog/Log.h"

int main(int argc, char *argv[])
{
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender("./test.txt", 8000000, 1);
    plog::init(plog::debug, &consoleAppender).addAppender(&fileAppender);

    QApplication a(argc, argv);

    std::shared_ptr<Decoder> decoder = std::make_shared<Decoder>();
    Webrtc::getInstance()->setDecoder(decoder);

    MainWindow w;
    QObject::connect(decoder.get(), &Decoder::signalNewFrame, &w, &MainWindow::slotShowVideo);

    w.show();

    return a.exec();
}
