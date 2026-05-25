#include "decoder.h"
#include "plog/Log.h"
#include <utils.h>

const int RTP_HEADER_SIZE = 12;
const uint8_t START_CODE[4] = {0x00, 0x00, 0x00, 0x01};

Decoder::Decoder()
{
    const AVCodec* codec = avcodec_find_decoder_by_name("h264_cuvid");
    if (codec == nullptr) {
        PLOGD << "avcodec_find_decoder error";
        return;
    }

    mCodecCtx = avcodec_alloc_context3(codec);
    if (mCodecCtx == nullptr) {
        PLOGD << "avcodec alloc fail.";
        return;
    }

    if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0) < 0) {
        PLOGD<< "Failed to create HW device context";
        return;
    }

    mCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
        PLOGD <<" Failed to open codec";
        return;
    }

    mSwsCtx = sws_getContext(mWidth, mHeight, AV_PIX_FMT_NV12, mWidth, mHeight, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    mHwFrame = av_frame_alloc();
    mSwFrame = av_frame_alloc();
    mPacket = av_packet_alloc();

    mNalu.resize(4 * 1024 * 1024);
}

Decoder::~Decoder()
{
    av_frame_free(&mHwFrame);
    av_frame_free(&mSwFrame);
    av_packet_free(&mPacket);
    sws_freeContext(mSwsCtx);
    avcodec_free_context(&mCodecCtx);
}

void Decoder::praseFrame(AVFrame* avFrame)
{
    VideoFrame *videoFrame = new VideoFrame;
    videoFrame->data.resize(mWidth * mHeight * 3 / 2); // 保证 mWidth * mHeight 一定偶数
    uint8_t *dstY  = videoFrame->data.data();
    uint8_t *dstUV = videoFrame->data.data() + mWidth * mHeight;
    for (int i = 0; i < mHeight; ++i) {
        memcpy(dstY + i * mWidth, avFrame->data[0] + i * avFrame->linesize[0], mWidth);
    }

    for (int i = 0; i < mHeight / 2; ++i) {
        memcpy(dstUV + i * mWidth, avFrame->data[1] + i * avFrame->linesize[1], mWidth);
    }

    videoFrame->height = mHeight;
    videoFrame->width = mWidth;
    emit signalNewFrame(videoFrame);
}

// 解码大概花费6ms, 30fps 足够用了
int Decoder::decode(uint8_t *data, int size)
{
    uint8_t *payload = data + RTP_HEADER_SIZE;
    int payloadLength = size - RTP_HEADER_SIZE;
    uint8_t naluType = payload[0] & 0x1F;
    uint8_t *nalu = mNalu.data();

    if (naluType >= 1 && naluType <= 23) {
        memcpy(nalu, (char *)START_CODE, 4);
        memcpy(nalu + 4, payload, payloadLength);
        mNaluLength = payloadLength + 4;
    } else if (naluType == 28) {
        // FU-A
        uint8_t fuIndicator = payload[0];
        uint8_t fuHeader = payload[1];
        bool start = fuHeader & 0x80;
        bool end = fuHeader & 0x40;
        uint8_t naluHeader = (fuIndicator & 0xE0) | (fuHeader & 0x1F);

        if (start) {
            memcpy(nalu, (char *)START_CODE, 4);
            nalu[4] = naluHeader;
            mNaluLength = 5;
        }

        memcpy(nalu + mNaluLength, payload + 2, payloadLength - 2);
        mNaluLength += payloadLength - 2;

        if (!end) {
            return 0;
        }
    }

    mPacket->data = nalu;
    mPacket->size = mNaluLength;
    mNaluLength = 0;
    int ret = avcodec_send_packet(mCodecCtx, mPacket);
    if (ret < 0) {
        av_packet_unref(mPacket);
        PLOGD << " avcodec_send_packet fail,  ret = " << ret;
        return -1;
    }

    ret = avcodec_receive_frame(mCodecCtx, mHwFrame);
    if (ret < 0) {
        av_packet_unref(mPacket);
        PLOGD << " avcodec_receive_frame fail, ret = " << ret;
        return -1;
    }

    mSwFrame->format = AV_PIX_FMT_NV12;
    mSwFrame->width  = mHwFrame->width;
    mSwFrame->height = mHwFrame->height;

    if (av_frame_get_buffer(mSwFrame, 32) < 0) {
        PLOGD << "Failed to alloc buffer for sw_frame";
        return -1;
    }

    ret = av_hwframe_transfer_data(mSwFrame, mHwFrame, 0);
    if (ret < 0) {
        PLOGD <<  "av_hwframe_transfer_data fail. ";
        return -1;
    }

    static int frameCount = 0;
    PLOGD << "Frame " << frameCount++;
    praseFrame(mSwFrame);
    av_packet_unref(mPacket);
    return 0;
}
