HEADERS =   config.h \
  machineprocess.h \
  machinetab.h \
  wizard.h \
  machinewizard.h \
  helpwindow.h \
  configwindow.h \
  mainwindow.h \
  vnc/remoteview.h\
  vnc/vncclientthread.h\
  vnc/vncview.h \
  machineview.h \
  machinesplash.h \
  machineconfig.h \
  machineconfigobject.h \
   settingstab.h \
   qtemuenvironment.h \
   harddiskmanager.h \
   controlpanel.h

SOURCES =   main.cpp \
  mainwindow.cpp \
  helpwindow.cpp \
  configwindow.cpp \
  machineprocess.cpp \
  wizard.cpp \
  machinewizard.cpp \
  machinetab.cpp \
  vnc/remoteview.cpp\
  vnc/vncclientthread.cpp\
  vnc/vncview.cpp \
  machineview.cpp \
  machinesplash.cpp \
  machineconfig.cpp \
  machineconfigobject.cpp \
   settingstab.cpp \
   qtemuenvironment.cpp \
   harddiskmanager.cpp \
   controlpanel.cpp

RESOURCES =   qtemu.qrc
QT +=   xml svg webkit
win32 {
    LIBS =     -lshell32
    RC_FILE =     qtemu.rc
}
TEMPLATE =   app
TRANSLATIONS =   translations/template_qtemu.ts \
  translations/qtemu_de.ts \
  translations/qtemu_tr.ts \
  translations/qtemu_ru.ts \
  translations/qtemu_cz.ts \
  translations/qtemu_fr.ts \
  translations/qtemu_it.ts \
  translations/qtemu_es.ts \
  translations/qtemu_pt-BR.ts \
  translations/qtemu_pl.ts
CONFIG +=   debug_and_release

LIBS += -lvncclient
DEFINES += QTONLY

DISTFILES += TODO CHANGELOG README

FORMS += \
 ui/settingstab.ui \
 ui/controlpanel.ui

