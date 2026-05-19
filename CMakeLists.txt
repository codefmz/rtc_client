cmake_minimum_required(VERSION 3.16)

project(rtc_client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS CoreTools)
find_package(plog CONFIG REQUIRED)
find_package(LibDataChannel CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/include
)

qt_standard_project_setup() # set CMAKE_AUTOMOC, CMAKE_AUTOUIC true

qt_add_resources(rtc_client
    PREFIX "/images"
    FILES res/img/video.png res/img/video.svg
)

# 带有 QOBJECT 的类必须在 qt_add_executable 中列出，否则会出现 moc 相关的链接错误
qt_add_executable(rtc_client
    ui/mainwindow.ui

    src/ip_edit.h
    src/ip_edit.cpp

    src/mainwindow.h
    src/mainwindow.cpp

    src/videoplayerwidget.h
    src/videoplayerwidget.cpp

    src/decoder.h
    src/decoder.cpp

    src/videowidget.h
    src/videowidget.cpp

    src/utils.cpp
    src/webrtc.cpp
    src/main.cpp
)
target_link_directories(rtc_client PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(rtc_client PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::OpenGLWidgets plog::plog LibDataChannel::LibDataChannel ${OpenCV_LIBS} )
target_link_libraries(rtc_client PRIVATE avformat avcodec avutil swscale)
target_compile_definitions(rtc_client PRIVATE RES_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(rtc_client PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(WIN32)
    add_custom_command(TARGET rtc_client POST_BUILD
        COMMAND Qt6::windeployqt
            --dir "${PROJECT_SOURCE_DIR}/bin"
            "$<TARGET_FILE:rtc_client>"
        COMMENT "Deploying Qt runtime libraries to bin"
    )
endif()
