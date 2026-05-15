#include "utils.h"
#include <memory>
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QDateTime>
#include <mutex>
#include <fstream>
#include <QDebug>

const char * Utils::QTLOG_FILE_PATH = "rtc_client.log";

QString Utils::SYS_DIR = "";
QString Utils::USR_DIR = "";

bool Utils::okToDelete(QWidget *parent, const QString &title, const QString &text, const QString &detailedText)
{
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
    if (parent) {
        messageBox->setWindowModality(Qt::WindowModal);
    }

    messageBox->setIcon(QMessageBox::Question);
    messageBox->setWindowTitle(QString("%1").arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty()) {
        messageBox->setInformativeText(detailedText);
    }

    QPushButton *deleteButton = messageBox->addButton(QObject::tr("&Delete"), QMessageBox::AcceptRole);
    messageBox->addButton(QObject::tr("Do & Not Delete"), QMessageBox::RejectRole);
    messageBox->setDefaultButton(deleteButton);
    messageBox->exec();
    return messageBox->clickedButton() == deleteButton;
}

void Utils::information(QWidget *parent, const QString &title, const QString &text, const QString &detailedText)
{
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
    if (parent) {
        messageBox->setWindowModality(Qt::WindowModal);
    }

    messageBox->setWindowTitle(QString("%1").arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty()) {
        messageBox->setInformativeText(detailedText);
    }

    messageBox->setIcon(QMessageBox::Information);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->exec();
}

void Utils::warning(QWidget *parent, const QString &title, const QString &text, const QString &detailedText)
{
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
    if (parent) {
        messageBox->setWindowModality(Qt::WindowModal);
    }

    messageBox->setWindowTitle(QString("%1").arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty()) {
        messageBox->setInformativeText(detailedText);
    }

    messageBox->setIcon(QMessageBox::Warning);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->exec();
}

void Utils::critical(QWidget *parent, const QString &title, const QString &text, const QString &detailedText)
{
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
    if (parent) {
        messageBox->setWindowModality(Qt::WindowModal);
    }

    messageBox->setWindowTitle(QString("%1").arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty()) {
        messageBox->setInformativeText(detailedText);
    }

    messageBox->setIcon(QMessageBox::Critical);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->exec();
}


static std::mutex g_logMutex;
static int g_logDest;

void Utils::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const char *typeStr = nullptr;

    switch (type) {
        case QtDebugMsg:
            typeStr = "DEBUG";
            break;
        case QtInfoMsg:
            typeStr = "INFO";
            break;
        case QtWarningMsg:
            typeStr = "WARN";
            break;
        case QtCriticalMsg:
            typeStr = "ERROR";
            break;
        case QtFatalMsg:
            typeStr = "FATAL";
            break;
        default:
            typeStr = "";
            break;
    }

    if (g_logDest & static_cast<int>(Utils::QTLOG_DEST::CONSOLE)) {
        fprintf(stdout, "[%s] [%s] (%s:%u) %s \n", typeStr, time.toUtf8().constData(), context.file, context.line, localMsg.constData());
        fflush(stdout);
    }

    if (g_logDest & static_cast<int>(Utils::QTLOG_DEST::FILE)) {
        std::lock_guard<std::mutex> lock(g_logMutex);
        FILE* pf = fopen(Utils::QTLOG_FILE_PATH, "a+");
        if (pf != nullptr) {
            fprintf(pf, "[%s] [%s] (%s:%u) %s \n", typeStr, time.toUtf8().constData(), context.file, context.line, localMsg.constData());
            fflush(pf);
            fclose(pf);
        }
    }
}

void Utils::initQtLog(QTLOG_DEST dest)
{
    g_logDest = static_cast<int>(dest);
    qInstallMessageHandler(Utils::customMessageHandler);
}

cv::Mat Utils::QImageToMat(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_RGB888: {
        // QImage 是 RGB 顺序，而 OpenCV 默认是 BGR 顺序
        cv::Mat mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        cv::Mat matBGR;
        cv::cvtColor(mat, matBGR, cv::COLOR_RGB2BGR);
        return matBGR; // clone 防止悬空指针
    }
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied: {
        cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        return mat.clone();
    }
    case QImage::Format_Grayscale8: {
        cv::Mat mat(image.height(), image.width(), CV_8UC1,
                    const_cast<uchar*>(image.bits()), image.bytesPerLine());
        return mat.clone();
    }
    default:
        // 不支持的格式，转换成 RGB888 再处理
        QImage converted = image.convertToFormat(QImage::Format_RGB888);
        return QImageToMat(converted);
    }
}

void Utils::saveRTPBuf(char *buf, size_t size)
{
    std::ofstream fs("E:/camrtp.rtp", std::ios::binary | std::ios::app);
    if (fs.is_open()) {
        fs.write(buf, size);
    }
}

const std::string filePath = "./test22264.264";

void Utils::dumpToFile(uint8_t *data, int length)
{
    std::ofstream file(filePath, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        qDebug() << "open file failed, errno = " << errno;
        return;
    }

    file.write((char*)data, length);
    file.close();
}

const int RTP_HEADER_SIZE = 12;
const uint8_t START_CODE[4] = {0x00, 0x00, 0x00, 0x01};


const std::string h264FilePath = "./test.264";

bool Utils::parseRTPtoH264(uint8_t *data, int length)
{
    std::ofstream fout(h264FilePath, std::ios::binary | std::ios::app);
    if (!fout) {
        qDebug() << "open file error";
        return false;
    }

    const uint8_t *payload = data + RTP_HEADER_SIZE;
    size_t payloadLength = length - RTP_HEADER_SIZE;
    uint8_t naluType = payload[0] & 0x1F;
    if (naluType >= 1 && naluType <= 23) {
        fout.write((char *)START_CODE, 4);
        fout.write((char *)payload, payloadLength);
    } else if (naluType == 28) {
        // FU-A
        uint8_t fuIndicator = payload[0];
        uint8_t fuHeader = payload[1];
        bool start = fuHeader & 0x80;
        bool end = fuHeader & 0x40;
        uint8_t naluHeader = (fuIndicator & 0xE0) | (fuHeader & 0x1F);

        if (payloadLength < 2) {
            qDebug() << "payloadLength < 2";
            return false;
        }

        if (start) {
            fout.write((char *)START_CODE, 4);
            fout.write((char *)&naluHeader, 1);
        }
        fout.write((char *)(payload + 2), payloadLength - 2);
    }

    return true;
}
