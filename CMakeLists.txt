# Generated from mediacenter.pro.

cmake_minimum_required(VERSION 3.16)
project(mediacenter VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# enable C++ 23 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui SerialPort Widgets HttpServer NetworkAuth)

qt_add_executable(mediacenter WIN32 MACOSX_BUNDLE
    beamercontrol.cpp beamercontrol.h beamercontrol.ui
    configuredmx.cpp configuredmx.h configuredmx.ui
    debugwindow.cpp debugwindow.h debugwindow.ui
    dmxthread.cpp dmxthread.h
    eurolitepmd8configuration.cpp eurolitepmd8configuration.h eurolitepmd8configuration.ui
    lightbars.cpp lightbars.h lightbars.ui
    lightfader.cpp lightfader.h lightfader.ui
    lightpresets.cpp lightpresets.h lightpresets.ui
    main.cpp
        Mediacenter.cpp Mediacenter.h mediacenter.ui
    preset.cpp preset.h preset.ui
        livestream.h livestream.cpp
)
target_link_libraries(mediacenter PUBLIC
    Qt::Core
    Qt::Gui
    Qt::SerialPort
    Qt::Widgets
    Qt::HttpServer
    Qt6::NetworkAuth
)


# Resources:
set(mediacenter_resource_files
    "off.xpm"
    "on.xpm"
)

qt6_add_resources(mediacenter "mediacenter"
    PREFIX
        "/"
    FILES
        ${mediacenter_resource_files}
)

if(WIN32)
    add_definitions(-DFTD2XX_STATIC)
    target_link_libraries(mediacenter PUBLIC
            ${CMAKE_SOURCE_DIR}/ftd2xx.lib
    )
    qt_generate_deploy_app_script(
            TARGET mediacenter
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()

install(TARGETS mediacenter
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
