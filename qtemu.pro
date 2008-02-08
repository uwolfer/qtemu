HEADERS =   config.h \
  machineprocess.h \
  machinetab.h \
  wizard.h \
  machinewizard.h \
  helpwindow.h \
  configwindow.h \
  mainwindow.h
SOURCES =   main.cpp \
  mainwindow.cpp \
  helpwindow.cpp \
  configwindow.cpp \
  machineprocess.cpp \
  wizard.cpp \
  machinewizard.cpp \
  machinetab.cpp
RESOURCES =   qtemu.qrc
QT +=   xml
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
