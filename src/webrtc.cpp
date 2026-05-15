#include "webrtc.h"
#include "plog/Log.h"
#include <iostream>
#include <memory>
#include <future>
#include <nlohmann/json.hpp>
#include <utils.h>
#include "RtcPacket.h"
#include <QDebug>

#pragma comment(lib, "ws2_32.lib")

using namespace rtc;
using namespace std;
using nlohmann::json;

// Create and setup a PeerConnection
shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Configuration &config, weak_ptr<rtc::WebSocket> wws, std::string id) {
    auto pc = std::make_shared<rtc::PeerConnection>(config);
    pc->onStateChange([](rtc::PeerConnection::State state) {
        std::cout << "State: " << state << std::endl;
    });

    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        std::cout << "Gathering State: " << state << std::endl;
    });

    pc->onLocalDescription([wws, id](rtc::Description description) {
        json message = {{"id", id},
                        {"type", description.typeString()},
                        {"description", std::string(description)}};

        if (auto ws = wws.lock()) {
            ws->send(message.dump());
        }
    });

    pc->onLocalCandidate([wws, id](rtc::Candidate candidate) {
        json message = {{"id", id},
                        {"type", "candidate"},
                        {"candidate", std::string(candidate)},
                        {"mid", candidate.mid()}};

        if (auto ws = wws.lock()) {
            ws->send(message.dump());
        }
    });

    return pc;
};

int Webrtc::connectServer(const std::string &ipAddr)
{
    std::string localId = Utils::randomId(4);
    WebSocket::Configuration config;
    config.disableTlsVerification = true;

    auto ws = make_shared<WebSocket>(std::move(config));
    std::promise<void> wsPromise;
    auto wsFuture = wsPromise.get_future();
    ws->onOpen([&wsPromise]() {
        qDebug() << " open ws success.";
        wsPromise.set_value();
    });

    ws->onError([&wsPromise](std::string s) {
        qDebug() << "WebSocket error, info : " << s;
        wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    ws->onClosed([]() {
        qDebug() << "Client WebSocket: Closed";
    });

    ws->onMessage([this](variant<binary, string> message) {
        if (holds_alternative<string>(message)) {
            qInfo() << "WebSocketServer: Received string message: " << std::get<string>(message);
            json data = json::parse(std::get<std::string>(message));
            auto it = data.find("id");
            if (it == data.end()) {
                return;
            }

            auto id = it->get<std::string>();
            it = data.find("type");
            if (it == data.end()) {
                return;
            }

            auto type = it->get<std::string>();
            if (type == "offer" || type == "answer") {
                auto sdp = data["description"].get<std::string>();
                pc->setRemoteDescription(rtc::Description(sdp, type));
            } else if (type == "candidate") {
                auto sdp = data["candidate"].get<std::string>();
                auto mid = data["mid"].get<std::string>();
                pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
            }
        } else {
            qDebug() << " websocket received bytes.";
        }
    });

    const std::string url =  "ws://" + ipAddr + "/" + localId;
    try {
        ws->open(url);
        wsFuture.get(); //等待websocket 连接成功
    } catch (const std::exception& ex) {
        qDebug() << "Future raised exception: " << ex.what();
        return -1;
    }

    rtc::Configuration rtcConfig;
    pc = createPeerConnection(rtcConfig, ws, localId);


    pc->onTrack([this](shared_ptr<Track> t) {
        string mid = t->mid();
        qDebug() << "Track 2: Received track with mid = " << mid;
        if (mid != "video") {
            qDebug() << "Wrong track mid";
            return;
        }

        t->onOpen([mid]() {
            qDebug() << "Track 2: Track with mid = " << mid << " opened.";
        });

        t->onClosed([this, mid]() {
            qDebug() << "Track 2: Track with mid = " << mid << " closed.";
            track = nullptr;
        });

        t->onMessage([this](rtc::binary message) {
            if (mIsSend) {
                mDecoder->decode(reinterpret_cast<uint8_t *>(message.data()), message.size());
            }
        }, nullptr);

        std::atomic_store(&track, t);
    });

    const std::string label = "rtc_ctl";
    dataDc = pc->createDataChannel(label);
    wsPromise = std::promise<void>();
    wsFuture = wsPromise.get_future();

    dataDc->onOpen([&wsPromise]() {
        qDebug() << "DataChannel from server open.";
        wsPromise.set_value();
    });

    dataDc->onError([&wsPromise](std::string s) {
        qFatal() << "WebSocket error, info : " << s;
        wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    dataDc->onClosed([this]() {
        qDebug() << "DataChannel from server closed.";
        dataDc = nullptr;
    });

    dataDc->onMessage([](auto data) {
        Webrtc::getInstance()->processResponse(data);
    });

    try {
        wsFuture.get(); //等到 data channel 连接成功
    } catch (const std::exception& ex) {
        PLOG_ERROR << "Future raised exception: " << ex.what();
        return -1;
    }

    return 0;
}

void Webrtc::disConnectServer()
{
    dataDc->close();
    pc->close();
    pc = nullptr;
}

int Webrtc::sendMessage(RTC_CMD cmd, int timeoutMs, uint8_t *in, int len, uint8_t** out, int *outLen)
{
    if (dataDc == nullptr) {
        return -1;
    }

    input->cmd = cmd;
    input->len = INPUT_HEADER_SIZE + len;
    memcpy(input->buf, in, len);
    dataDc->send((const rtc::byte *)input, input->len);
    {
        std::unique_lock<std::mutex> lock(resMtx);
        bool waited = resCond.wait_for(lock, std::chrono::seconds(timeoutMs), [this]() {
            return !resData.empty();
        });

        if (!waited) { /* 没等到表示超时了 */
            qDebug() << "DataChannel recv time out, message : ";
            return -2;
        }

        memcpy(output, resData.data(), resData.size());
        resData.clear();
    }

    if (out != nullptr) {
        *out = output->buf;
        *outLen = output->len - OUTPUT_HEADER_SIZE;
    }

    return output->ret;
}


void Webrtc::processResponse(rtc::message_variant &data)
{
    {
        std::lock_guard<std::mutex> lock(resMtx);
        resData = std::move(std::get<binary>(data));
    }

    RTC_Output *output = (RTC_Output*)resData.data();
    if (output->ret == RET_KLIPPY_POS) {
        RTC_Pos_Param *param = (RTC_Pos_Param *)(output->buf);
        if (posChangeCallback) {
            posChangeCallback(param->x, param->y, param->z);
        }
        qDebug() << "process klippy pos ret ";
        resData.clear();
    } else {
        resCond.notify_all();
    }
}

void Webrtc::startShowVideo()
{
    mIsSend = true;
}

void Webrtc::stopShowVideo()
{
    mIsSend = false;
}

Webrtc::Webrtc() : track(nullptr)
{
    input = new RTC_Input;
    output = new RTC_Output;
    mIsSend = false;
}

Webrtc::~Webrtc()
{
    delete input;
    delete output;
}
