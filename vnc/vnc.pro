#TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lvncclient
DEFINES += QTONLY

HEADERS += remoteview.h\
           vncclientthread.h\
           vncview.h
SOURCES += remoteview.cpp\
           vncclientthread.cpp\
           vncview.cpp
#CONFIG +=   debug_and_release