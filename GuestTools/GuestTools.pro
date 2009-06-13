PROJECT = GuestTools
TEMPLATE = app

# DEPENDPATH += .
INCLUDEPATH += qextserialport/.
QMAKE_LIBDIR += qextserialport/build

QT += core \
    gui

HEADERS += guesttools.h \
    modules/guestmodule.h

SOURCES += main.cpp \
    guesttools.cpp \

FORMS += guesttools.ui \

RESOURCES += resources/resources.qrc

CONFIG(debug, debug|release):LIBS += -lqextserialport
else:LIBS += -lqextserialport

SOURCES += modules/clipboard/clipboardsync.cpp \
    modules/guestmodule.cpp

HEADERS += modules/clipboard/clipboardsync.h

CONFIG += qt
CONFIG += warn_on
CONFIG += thread
unix:DEFINES += _TTY_POSIX_
win32:DEFINES += _TTY_WIN_
