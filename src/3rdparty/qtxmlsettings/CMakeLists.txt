cmake_minimum_required(VERSION 2.8)

project(qtxmlsettings)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

find_package(Qt4 REQUIRED)
set(QT_DONT_USE_QTGUI  TRUE)
set(QT_USE_QTXML TRUE)
include(${QT_USE_FILE})

set(SOURCES qtxmlsettings.cpp)
set(HEADERS qtxmlsettings.h)

qt4_wrap_cpp(MOC_SOURCES ${HEADERS})

qt4_wrap_ui(UI_HEADERS ${FORMS})

qt4_add_resources(QRC_SOURCES ${RESOURCES})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${UI_HEADERS} ${QRC_SOURCES})

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} )

