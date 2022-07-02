cmake_minimum_required(VERSION 3.5)

project(minic_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-mwindows")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

# minic 编译器源码
include_directories( ${PROJECT_SOURCE_DIR}/minic/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/minic/src MINIC)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    minic.cpp
    minic.h
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(minic_gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${MINIC}
        )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET minic_gui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
        add_executable(minic_gui WIN32
            ${PROJECT_SOURCES}
            ${MINIC}
            )
endif()

target_link_libraries(minic_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(minic_gui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(minic_gui)
endif()
