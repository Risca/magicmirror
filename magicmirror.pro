TEMPLATE = app

CONFIG += gui network debug core

QT += widgets network

OBJECTS_DIR = .obj
MOC_DIR = .moc

VERSION = 1.7.4
DEFINES += VERSION_STRING=\\\"$${VERSION}\\\"

*-g++ {
    GCC_VERSION = $$system("$${QMAKE_CXX} -dumpversion")
    # The mangling of 'va_list' changed in GCC 4.4 (for arm), but it seem to
    # work anyway, so we Suppress the warning on GCC 4.4 and later
    !contains(GCC_VERSION, [1-4].[0-3]) {
        QMAKE_CXXFLAGS += -Wno-psabi
    }
}

SOURCES = MirrorFrame.cpp \
	WeatherData.cpp \
    calendar/calendarfactory.cpp \
    calendar/fakedata.cpp \
    calendar/icscalendar.cpp \
    domoticz/sensor.cpp \
	main.cpp \
    settingsfactory.cpp \
        weathericon.cpp
		
HEADERS = MirrorFrame.h \
	WeatherData.h \
    calendar/calendarinterface.h \
    calendar/fakedata.h \
    calendar/icscalendar.h \
    domoticz/sensor.h \
    settingsfactory.h \
        weathericon.h

FORMS += \
    MirrorFrame.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libical
