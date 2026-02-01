TARGET = magicmirror
TEMPLATE = app

CONFIG += gui network debug core

CONFIG += c++11

QT += widgets network

unix {
  target.path = /data/dev
  INSTALLS += target
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += GLM_ENABLE_EXPERIMENTAL

OBJECTS_DIR = .obj
MOC_DIR = .moc

VERSION = 1.17.0
DEFINES += VERSION_STRING=\\\"$${VERSION}\\\" \
    GL_GLEXT_PROTOTYPES \
    GLM_FORCE_RADIANS

*-g++ {
    GCC_VERSION = $$system("$${QMAKE_CXX} -dumpversion")
    # The mangling of 'va_list' changed in GCC 4.4 (for arm), but it seem to
    # work anyway, so we Suppress the warning on GCC 4.4 and later
    !contains(GCC_VERSION, [1-4].[0-3]) {
        QMAKE_CXXFLAGS += -Wno-psabi
    }
}

INCLUDEPATH += main \
    utils \
    widgets\
    widgets/calendar \
    widgets/weather

SOURCES = main/MirrorFrame.cpp \
        main/main.cpp \
    utils/effects.cpp \
    utils/qrcodepopup.cpp \
	utils/settingsfactory.cpp \
        utils/iconcache.cpp \
    widgets/calendar/customcalendarwidget.cpp \
    widgets/calendar/data_sources/calendarsourcefactory.cpp \
    widgets/calendar/data_sources/googlecalendarsource.cpp \
    widgets/calendar/data_sources/icssource.cpp \
    widgets/calendar/calendar.cpp \
    widgets/compliment.cpp \
    widgets/departures/data_sources/resrobotdepartures.cpp \
    widgets/departures/departurewidget.cpp \
    widgets/departures/singledeparturewidget.cpp \
    widgets/fadingqlabel.cpp \
    widgets/schedule/schedule.cpp \
    widgets/sensors/data_sources/domoticzsensor.cpp \
    widgets/sensors/data_sources/sensormodel.cpp \
    widgets/sensors/data_sources/sensorsourcefactory.cpp \
    widgets/sensors/sensors.cpp \
    widgets/slideshow.cpp \
    widgets/weather/currentconditions.cpp \
    widgets/weather/data_sources/fakeforecastdatasource.cpp \
    widgets/weather/data_sources/fakeweatherconditionsdatasource.cpp \
    widgets/weather/data_sources/forecastdatamodel.cpp \
    widgets/weather/data_sources/openweathermapconditionsdatasource.cpp \
    widgets/weather/data_sources/openweathermapforecastdatasource.cpp \
    widgets/weather/data_sources/weathersourcefactory.cpp \
    widgets/weather/forecast.cpp \
    widgets/weather/globe.cpp \
    widgets/weather/primitives/shapegenerator.cpp

HEADERS = main/MirrorFrame.h \
    utils/effects.h \
    utils/formatting.h \
    utils/qrcodepopup.h \
    utils/sensordata.h \
    utils/settingsfactory.h \
    utils/iconcache.h \
    widgets/calendar/calendar.h \
    widgets/calendar/customcalendarwidget.h \
    widgets/calendar/data_sources/cal_isource.h \
    widgets/calendar/data_sources/cal_nosource.h \
    widgets/calendar/data_sources/event.hpp \
    widgets/calendar/data_sources/googlecalendarsource.h \
    widgets/calendar/data_sources/icssource.h \
    widgets/compliment.h \
    widgets/departures/data_sources/departure.h \
    widgets/departures/data_sources/departure_isource.h \
    widgets/departures/data_sources/resrobotdepartures.h \
    widgets/departures/departurewidget.h \
    widgets/departures/singledeparturewidget.h \
    widgets/fadingqlabel.h \
    widgets/schedule/schedule.h \
    widgets/sensors/data_sources/domoticzsensor.h \
    widgets/sensors/data_sources/sensor_isource.h \
    widgets/sensors/data_sources/sensor_nosource.h \
    widgets/sensors/data_sources/sensormodel.h \
    widgets/sensors/sensors.h \
    widgets/slideshow.h \
    widgets/weather/currentconditions.h \
    widgets/weather/data_sources/fakeforecastdatasource.h \
    widgets/weather/data_sources/fakeweatherconditionsdatasource.h \
    widgets/weather/data_sources/forecastdatamodel.h \
    widgets/weather/data_sources/icurrentconditionsdatasource.h \
    widgets/weather/data_sources/iforecastdatasource.h \
    widgets/weather/data_sources/openweathermapconditionsdatasource.h \
    widgets/weather/data_sources/openweathermapforecastdatasource.h \
    widgets/weather/forecast.h \
    widgets/weather/globe.h \
    widgets/weather/primitives/shapedata.h \
    widgets/weather/primitives/shapegenerator.h \
    widgets/weather/primitives/vertex.h


FORMS += \
    main/MirrorFrame.ui \
    widgets/calendar/calendar.ui \
    widgets/departures/singledeparturewidget.ui \
    widgets/schedule/schedule.ui \
    widgets/sensors/sensors.ui \
    widgets/weather/currentconditions.ui \
    widgets/weather/forecast.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libical o2 libqrencode

RESOURCES += main/stylesheets/breeze.qrc \
    resources/icons.qrc \
    resources/images.qrc \
    widgets/weather/shaders.qrc

DISTFILES += \
    widgets/weather/frag.fsh \
    widgets/weather/vert.vsh
