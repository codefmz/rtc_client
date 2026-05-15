#ifndef WEBRTC_H
#define WEBRTC_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "RtcPacket.h"
#include "rtc/rtc.hpp"

#include "decoder.h"

class Webrtc {
public:
    static Webrtc *getInstance() {
        static Webrtc instance;
        return &instance;
    }
    Webrtc(const Webrtc &) = delete;
    Webrtc &operator=(const Webrtc &) = delete;
    Webrtc(Webrtc &&) = delete;
    Webrtc &operator=(Webrtc &&) = delete;

    int connectServer(const std::string &ipAddr);
    void disConnectServer();
    int sendMessage(RTC_CMD cmd, int timeoutMs = 3, uint8_t *in = nullptr, int len = 0, uint8_t** out = nullptr, int *outLen = nullptr);
    void processResponse(rtc::message_variant &data);

    void onPosChangeCallback(std::function<void(double, double, double)> callback) {
        this->posChangeCallback = callback;
    }

    void startShowVideo();
    void stopShowVideo();

    void setDecoder(std::shared_ptr<Decoder> decoder) {
        mDecoder = decoder;
    }

private:
    Webrtc();
    ~Webrtc();

private:
    std::function<void(double, double, double)> posChangeCallback;
    std::shared_ptr<rtc::Track> track;
    std::shared_ptr<rtc::DataChannel> dataDc;
    std::shared_ptr<rtc::PeerConnection> pc;
    std::mutex resMtx;
    std::condition_variable resCond;
    std::string trackMid;
    std::vector<std::byte> resData;
    RTC_Input *input;
    RTC_Output *output;

    std::atomic<bool> mIsSend;
    std::shared_ptr<Decoder> mDecoder;

};

#endif // WEBRTC_H
