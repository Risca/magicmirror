LIBTARGET = weather
BASEDIR   = $${PWD}
INCLUDEPATH += $${BASEDIR}
DEPENDPATH += $${BASEDIR}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../widgets/weather/release/ -lweather
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../widgets/weather/debug/ -lweather
else:unix: LIBS += -L$$OUT_PWD/../widgets/weather -lweather
