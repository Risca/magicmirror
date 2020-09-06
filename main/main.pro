CONFIG += gui network debug core

QT += widgets network

OBJECTS_DIR = .obj
MOC_DIR = .moc

VERSION = 1.7.5
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
    domoticz/sensor.cpp \
        main.cpp \
        weathericon.cpp

HEADERS = MirrorFrame.h \
        WeatherData.h \
    domoticz/sensor.h \
        weathericon.h

FORMS += \
    MirrorFrame.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libical

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../utils/release/ -lutils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
else:unix: LIBS += -L$$OUT_PWD/../utils/ -lutils

INCLUDEPATH += $$PWD/../utils
DEPENDPATH += $$PWD/../utils

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../widgets/calendar/release/ -lcalendar
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../widgets/calendar/debug/ -lcalendar
else:unix: LIBS += -L$$OUT_PWD/../widgets/calendar/ -lcalendar

INCLUDEPATH += $$PWD/../widgets/calendar
DEPENDPATH += $$PWD/../widgets/calendar

RESOURCES += stylesheets/breeze.qrc
