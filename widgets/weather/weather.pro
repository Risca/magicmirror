TEMPLATE = lib
CONFIG += staticlib

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../../utils/utils.pri)

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    currentconditions.cpp \
    data_sources/fakeforecastdatasource.cpp \
    data_sources/forecastdatamodel.cpp \
    data_sources/openweathermapconditionsdatasource.cpp \
    data_sources/openweathermapforecastdatasource.cpp \
    data_sources/sourcefactory.cpp \
    forecast.cpp

HEADERS += \
    currentconditions.h \
    data_sources/fakeforecastdatasource.h \
    data_sources/forecastdatamodel.h \
    data_sources/icurrentconditionsdatasource.h \
    data_sources/iforecastdatasource.h \
    data_sources/openweathermapconditionsdatasource.h \
    data_sources/openweathermapforecastdatasource.h \
    data_sources/weatherdata.h \
    forecast.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    currentconditions.ui \
    forecast.ui
