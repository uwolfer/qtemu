/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop@gmail.com>
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

#include "halobject.h"
#include "qtemuenvironment.h"
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QString>

QtEmuEnvironment::QtEmuEnvironment()
{
    getVersion();

    if(hal == 0)
        hal = new HalObject();
}

QtEmuEnvironment::~ QtEmuEnvironment()
{

}

void QtEmuEnvironment::getVersion()
{

    QSettings settings("QtEmu", "QtEmu");
    QString versionString;
    QProcess *findVersion = new QProcess();

#ifndef Q_OS_WIN32
    const QString qemuCommand = settings.value("command", "qemu").toString();
#elif defined(Q_OS_WIN32)
    const QString qemuCommand = settings.value("command", QCoreApplication::applicationDirPath() + "/qemu/qemu.exe").toString();
    QDir path(qemuCommand);
    path.cdUp();
    setWorkingDirectory(path->path());
#endif

    findVersion->start(qemuCommand, QStringList("--help"));
    findVersion->waitForFinished();

    if( findVersion->error() !=  QProcess::UnknownError )
    {
        qemuVersion[0] = -1;
        qemuVersion[1] = -1;
        qemuVersion[2] = -1;
        kvmVersion = -1;
        return;
    }
    
    QString infoString = findVersion->readLine();

    if( !infoString.contains("QEMU") )
    {
        qemuVersion[0] = -1;
        qemuVersion[1] = -1;
        qemuVersion[2] = -1;
        kvmVersion = -1;
        return;
    }

    QStringList infoStringList = infoString.split(' ');
    
    versionString = infoStringList.at(4);

    QStringList versionStringList = versionString.split('.');
    qemuVersion[0] = versionStringList.at(0).toInt();
    qemuVersion[1] = versionStringList.at(1).toInt();
    qemuVersion[2] = versionStringList.at(2).toInt();
    versionString = infoStringList.at(5);

    if(versionString.contains(QRegExp("kvm")))
    {
        kvmVersion = versionString.remove(QRegExp("\\D")).toInt();
        if(kvmVersion == 0)
            kvmVersion = -1; //indicates we think we've got kvm, but can't figure out what version
    }
    else
        kvmVersion = 0; //must be QEMU

    //grab the rest of the help text and search for kvm and kqemu options...

    QString helpString = findVersion->readAll();

    if(helpString.contains("kqemu"))
        supportsKqemu = true;
    else
        supportsKqemu = false;

    if(helpString.contains("kvm"))
        supportsKvm = true;
    else
        supportsKvm = false;

    delete findVersion;
    #ifdef Q_OS_WIN32
    delete path;
    #endif
    versionChecked = true;

    #ifdef DEVELOPER
    qDebug(("kvm: " + QString::number(kvmVersion) + " qemu: " + QString::number(qemuVersion[0]) + '.' + QString::number(qemuVersion[1]) + '.' + QString::number(qemuVersion[2])).toAscii());
    #endif
}

int * QtEmuEnvironment::getQemuVersion()
{
    if(!versionChecked)
        getVersion();
    return qemuVersion;
}

int QtEmuEnvironment::getKvmVersion()
{
    if(!versionChecked)
        getVersion();
    return kvmVersion;
}

HalObject* QtEmuEnvironment::getHal()
{
    if(hal == 0)
        hal = new HalObject();
    return hal;
}

bool QtEmuEnvironment::kvmSupport()
{
    if(!versionChecked)
        getVersion();
    return supportsKvm;
}

bool QtEmuEnvironment::kqemuSupport()
{
    if(!versionChecked)
        getVersion();
    return supportsKqemu;
}

HalObject* QtEmuEnvironment::hal = 0;
int QtEmuEnvironment::qemuVersion[] = {-1, -1, -1};
int QtEmuEnvironment::kvmVersion = -1;
bool QtEmuEnvironment::versionChecked = false;
bool QtEmuEnvironment::supportsKvm = false;
bool QtEmuEnvironment::supportsKqemu = false;
