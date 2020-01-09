TEMPLATE = app

CONFIG += gui network debug core

USE_RPI = FALSE

QT += widgets network

OBJECTS_DIR = .obj
MOC_DIR = .moc

defined (__USE_RPI__, var) {
     LIBS = -lwiringPi -lth02
    QMAKE_CXXFLAGS += -D__USE_RPI__
}

*-g++ {
    GCC_VERSION = $$system("$${QMAKE_CXX} -dumpversion")
    # The mangling of 'va_list' changed in GCC 4.4 (for arm), but it seem to
    # work anyway, so we Suppress the warning on GCC 4.4 and later
    !contains(GCC_VERSION, [1-4].[0-3]) {
        QMAKE_CXXFLAGS += -Wno-psabi
    }
}

SOURCES = MirrorFrame.cpp \
	CalendarData.cpp \
	WeatherData.cpp \
	main.cpp \
    settingsfactory.cpp \
        weathericon.cpp
		
HEADERS = MirrorFrame.h \
	CalendarData.h \
	WeatherData.h \
    settingsfactory.h \
        weathericon.h

FORMS += \
    MirrorFrame.ui

include(gitversion.pri)
