LIBTARGET = utils
BASEDIR   = $${PWD}
INCLUDEPATH += $${BASEDIR}
DEPENDPATH += $${BASEDIR}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../utils/release/ -lutils
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../utils/debug/ -lutils
else:unix: LIBS += -L$$OUT_PWD/../utils/ -lutils

