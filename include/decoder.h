#ifndef _DECODER_H_
#define _DECODER_H_

#include <opencv2/opencv.hpp>
#include <QObject>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/hwcontext.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}
#include <QImage>

class Decoder : public QObject{

    Q_OBJECT  // 必须添加
public:
    Decoder();
    ~Decoder();

    int decode(uint8_t *data, int size);

private:
    void praseFrame(AVFrame* frame);

signals:
    void signalNewFrame(QImage);

private:
    AVBufferRef* hw_device_ctx = nullptr;
    AVCodecContext *mCodecCtx;
    AVFrame *mHwFrame;
    AVFrame *mSwFrame;
    AVPacket *mPacket;
    SwsContext* mSwsCtx;
    int mWidth = 1920;
    int mHeight = 1080;

    std::vector<uint8_t> mNalu;
    uint32_t mNaluLength = 0;
};

#endif
