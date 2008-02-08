#/****************************************************************************
#**
#** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
#**
#** This file is part of QtEmu.
#**
#** This program is free software; you can redistribute it and/or modify
#** it under the terms of the GNU General Public License as published by
#** the Free Software Foundation; either version 2 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#** GNU General Public License for more details.
#**
#** You should have received a copy of the GNU Library General Public License
#** along with this library; see the file COPYING.LIB.  If not, write to
#** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#** Boston, MA 02110-1301, USA.
#**
#****************************************************************************/

PROJECT(qtemu)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

SET(qtemu_SRCS
    main.cpp
    mainwindow.cpp
    helpwindow.cpp
    configwindow.cpp
    machineprocess.cpp
    wizard.cpp
    machinewizard.cpp
    machinetab.cpp
)

SET(qtemu_MOC_HDRS
    config.h
    machineprocess.h
    machinetab.h
    wizard.h
    machinewizard.h
    helpwindow.h
    configwindow.h
    mainwindow.h
)

SET(qtemu_RCS
    qtemu.rc
)

SET(qtemu_RESOUCES
    qtemu.qrc
)

IF(MINGW)
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtemu.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/qtemu.rc
                             -o ${CMAKE_CURRENT_BINARY_DIR}/qtemu.o)
    SET(qtemu_SRCS ${qtemu_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/qtemu.o)
ELSE(MINGW)
    SET(qtemu_SRCS ${qtemu_SRCS} ${qtemu_RCS})
ENDIF(MINGW)

# verbose - on
# SET(CMAKE_VERBOSE_MAKEFILE ON)

# requires minimal Qt 4.1
SET(QT_MIN_VERSION "4.1.0")

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

# translation start
FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
)

# needed to create translation files
IF(NOT QT_LRELEASE_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Qt4 lrelease not found. Make sure that it has been built and installed by the Qt4 package.")
ENDIF(NOT QT_LRELEASE_EXECUTABLE)

MACRO(ADD_TRANSLATION_FILES _sources)
    FOREACH(_current_FILE ${ARGN})
        GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
        GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

        SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.qm)

        ADD_CUSTOM_COMMAND(
            OUTPUT ${_out}
            COMMAND ${QT_LRELEASE_EXECUTABLE}
            ARGS -verbose ${_in} -qm ${_out}
            DEPENDS ${_in}
        )

        SET(${_sources} ${${_sources}} ${_out})
   ENDFOREACH(_current_FILE)
ENDMACRO(ADD_TRANSLATION_FILES)


FILE(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/qtemu_*.ts)

ADD_TRANSLATION_FILES(QM_FILES ${TS_FILES})

# creating a custom target is needed to make the files build
# "ALL" means that it will be run by default
ADD_CUSTOM_TARGET(translations ALL
                   DEPENDS ${QM_FILES})

INSTALL (FILES ${QM_FILES}
         DESTINATION translations)
# translation end

SET(QT_USE_QTXML)

QT4_WRAP_CPP(qtemu_MOC_SRCS ${qtemu_MOC_HDRS})
QT4_ADD_RESOURCES (qtsourceview_RC_SRCS  ${qtemu_RESOUCES})
QT4_AUTOMOC(${qtemu_SRCS})

ADD_DEFINITIONS(
    -Wall
    ${QT_DEFINITIONS}
)

INCLUDE_DIRECTORIES(
    ${QT_INCLUDES}
    ${CMAKE_BINARY_DIR}
)

ADD_EXECUTABLE(qtemu
    ${qtemu_SRCS}
    ${qtemu_MOC_SRCS}
    ${qtsourceview_RC_SRCS}
)

TARGET_LINK_LIBRARIES(qtemu
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
)

FILE(GLOB helpfiles "${CMAKE_CURRENT_SOURCE_DIR}/help/*.png")
INSTALL(FILES ${helpfiles} DESTINATION help)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/help/main.htm DESTINATION help)

FILE(GLOB helpfiles_de "${CMAKE_CURRENT_SOURCE_DIR}/help/de/*.png")
INSTALL(FILES ${helpfiles_de} DESTINATION help/de)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/help/de/main.htm DESTINATION help/de)

INSTALL(TARGETS qtemu DESTINATION bin)
