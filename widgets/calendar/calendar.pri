LIBTARGET = calendar
BASEDIR   = $${PWD}
INCLUDEPATH += $${BASEDIR}
DEPENDPATH += $${BASEDIR}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../widgets/calendar/release/ -lcalendar
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../widgets/calendar/debug/ -lcalendar
else:unix: LIBS += -L$$OUT_PWD/../widgets/calendar/ -lcalendar
