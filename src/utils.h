#ifndef UTILS_H
#define UTILS_H

#include <string>            // std::string
#include <random>            // std::mt19937, std::uniform_int_distribution
#include <chrono>            // std::chrono::high_resolution_clock
#include <algorithm>         // std::generate
#include <memory>
#include <QWidget>
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <fstream>
#include <vector>

enum ItemUserDataRole {
    ItemImg = Qt::UserRole + 1,
    ItemPos
};

namespace Utils {

inline std::string randomId(size_t length) {
    using std::chrono::high_resolution_clock;
    static thread_local std::mt19937 rng(static_cast<unsigned int>(high_resolution_clock::now().time_since_epoch().count()));
    static const std::string characters("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::string id(length, '0');
    std::uniform_int_distribution<int> uniform(0, int(characters.size() - 1));
    std::generate(id.begin(), id.end(), [&]() {
        return characters.at(uniform(rng));
    });
    return id;
}

template <class T>
std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) {
    return ptr;
}

void information(QWidget *parent, const QString &title, const QString &text,
                 const QString &detailedText=QString());

void warning(QWidget *parent, const QString &title, const QString &text,
             const QString &detailedText=QString());

void critical(QWidget *parent, const QString &title, const QString &text,
             const QString &detailedText=QString());

bool question(QWidget *parent, const QString &title, const QString &text,
              const QString &detailedText = QString(),
              const QString &yesText = QObject::tr("&Yes"),
              const QString &noText = QObject::tr("&No"));

bool okToDelete(QWidget *parent, const QString &title, const QString &text,
                const QString &detailedText = QString());

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

cv::Mat QImageToMat(const QImage &image);

enum class QTLOG_DEST {
    NONE = 0,
    CONSOLE = 0x01,
    FILE = 0x02,
    CONSOLE_FILE = 0x03
};

extern const char *QTLOG_FILE_PATH;

void initQtLog(QTLOG_DEST dest);

void saveRTPBuf(char *buf, size_t size);

extern QString SYS_DIR;
extern QString USR_DIR;


void dumpToFile(uint8_t *data, int length);

bool parseRTPtoH264(uint8_t *data, int length);

}

typedef struct {
    int width;
    int height;
    std::vector<uint8_t> data;
} VideoFrame;

#endif // UTILS_H
