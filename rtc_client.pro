QT       += core gui opengl openglwidgets concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17
# CONFIG += absolute_path_includes
CONFIG -= depend_includepath #不要使用相对路径编译，编译 gstreamer 的时候会找不到头文件
QMAKE_CXXFLAGS += -DQT_NO_CPU_FEATURE_RDRND

DESTDIR = $$PWD/bin

SOURCES += \
    calibrt.cpp \
    camcalib.cpp \
    decoder.cpp \
    ip_edit.cpp \
    main.cpp \
    mainwindow.cpp \
    nozzle_calib.cpp \
    utils.cpp \
    webrtc.cpp

HEADERS += \
    RtcPacket.h \
    calibrt.h \
    camcalib.h \
    decoder.h \
    ip_edit.h \
    mainwindow.h \
    nozzle_calib.h \
    utils.h \
    webrtc.h

FORMS += \
    calibrt.ui \
    camcalib.ui \
    mainwindow.ui \
    nozzlecalib.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 如果是win32 增加宏定义 _WINSOCKAPI_ ARCH = "64-Bit" 宏
win32:DEFINES += _WINSOCKAPI_ ARCH='\\"64-Bit\\"'

INCLUDEPATH += $$PWD/include
message(INCLUDEPATH = $$INCLUDEPATH)

# 库路径
LIBS += -L$$PWD/lib

# 链接 GStreamer 库
LIBS +=  -lavformat \
        -lavcodec \
        -lavutil \
        -lswscale

# 链接其他依赖库
LIBS += -ldatachannel \
        -lopencv_world460d \
        -lnozzlecalib\
        -ljsoncpp\
        -lncnnd

RESOURCES += \
    res.qrc

