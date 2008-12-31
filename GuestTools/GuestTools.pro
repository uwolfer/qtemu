PROJECT = GuestTools
TEMPLATE = app
#DEPENDPATH += .
INCLUDEPATH += qextserialport/.
QMAKE_LIBDIR += qextserialport/build \
    build

QT += core \
    gui

HEADERS += modules/guestmodule.h \
    modules/clipboardsync.h \
    guesttools.h
SOURCES += modules/guestmodule.cpp \
    modules/clipboardsync.cpp \
    main.cpp \
    guesttools.cpp
FORMS += guesttools.ui
RESOURCES += resources/resources.qrc


CONFIG                 += qt
CONFIG                 += warn_on
CONFIG                 += thread

HEADERS                += qextserialport\qextserialbase.h \
                          qextserialport\qextserialport.h \
                          qextserialport\qextserialenumerator.h
SOURCES                += qextserialport\qextserialbase.cpp \
                          qextserialport\qextserialport.cpp \
                          qextserialport\qextserialenumerator.cpp

unix:HEADERS           += qextserialport\posix_qextserialport.h
unix:SOURCES           += qextserialport\posix_qextserialport.cpp
unix:DEFINES           += _TTY_POSIX_


win32:HEADERS          += qextserialport\win_qextserialport.h
win32:SOURCES          += qextserialport\win_qextserialport.cpp
win32:DEFINES          += _TTY_WIN_

win32:LIBS             += -lsetupapi
