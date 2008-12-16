
TEMPLATE = app
#DEPENDPATH += .
INCLUDEPATH += qextserialport/.
QMAKE_LIBDIR += qextserialport/build \
    build

QT += core \
    gui
#CONFIG += qt \
#    thread \
#    warn_on \
#    debug \
HEADERS += modules/guestmodule.h \
    modules/clipboardsync.h \
    guesttools.h
SOURCES += modules/guestmodule.cpp \
    modules/clipboardsync.cpp \
    main.cpp \
    guesttools.cpp
FORMS += guesttools.ui
RESOURCES += resources/resources.qrc
CONFIG(debug, debug|release):LIBS += -lqextserialportd
else:LIBS += -lqextserialport
unix:DEFINES = _TTY_POSIX_
win32:DEFINES = _TTY_WIN_