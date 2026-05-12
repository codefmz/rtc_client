cmake_minimum_required(VERSION 3.16)

project(helloworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS CoreTools)

qt_standard_project_setup()

qt_add_executable(helloworld
    mainwindow.h
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)

target_link_libraries(helloworld PRIVATE Qt6::Widgets)

set_target_properties(helloworld PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

if(WIN32)
    add_custom_command(TARGET helloworld POST_BUILD
        COMMAND Qt6::windeployqt
            --dir "${PROJECT_SOURCE_DIR}/bin"
            "$<TARGET_FILE:helloworld>"
        COMMENT "Deploying Qt runtime libraries to bin"
    )
endif()
