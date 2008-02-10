/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
**
** This file is part of QtEmu.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this library; see the file COPYING.LIB.  If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSettings>
#include <QFile>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtemu);

    QApplication app(argc, argv);

    //load translation
    QSettings settings("QtEmu", "QtEmu");
    QString locale = settings.value("language", QString(QLocale::system().name())).toString();
    QTranslator translator;
    QString path;
    //check for case when qtemu executable is in same dir (linux / win)
    path = QCoreApplication::applicationDirPath() + "/translations/qtemu_" + locale + ".qm";
    if (QFile::exists(path))
        translator.load(path);
    else
    {
        //check for case when qtemu executable is in bin/ (installed on linux)
        path = QCoreApplication::applicationDirPath() + "/../share/qtemu/translations/qtemu_" + locale + ".qm";
        if (QFile::exists(path))
            translator.load(path);
    }
    app.installTranslator(&translator);

    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
