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

#include "harddiskmanager.h"

#include <QTimer>
#include <QFileInfo>
#include <QProcess>

HardDiskManager::HardDiskManager(QObject *parent)
 : QObject(parent)
{
}


HardDiskManager::~HardDiskManager()
{
}

void HardDiskManager::upgradeImage()
{
    emit processingImage(true);

    upgradeImageName = currentImage.path() + '/' + currentImage.completeBaseName() + ".qcow";
#ifndef Q_OS_WIN32
    QString program = "qemu-img";
#elif defined(Q_OS_WIN32)
    QString program = "qemu/qemu-img.exe";
#endif
    QStringList arguments;
    currentProcess = new QProcess(this);

    connect(currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(upgradeComplete(int)));
    arguments << "convert" << currentImage.filePath() << "-O" << "qcow2" << upgradeImageName;
    currentProcess->start(program, arguments);
    updateProgressTimer = new QTimer(this);
     connect(updateProgressTimer, SIGNAL(timeout()), this, SLOT(updateUpgradeProgress()));
     updateProgressTimer->start(1000);
}

void HardDiskManager::upgradeComplete(int status)
{
    emit processingImage(false);
    if(status == 0)
    {
        setProperty("hdd", QVariant(upgradeImageName));
    }
    else
    {
        emit error(tr("Upgrading your hard disk image failed! Do you have enough disk space?<br />You may want to try upgrading manually using the program qemu-img."));
    }
}

void HardDiskManager::updateUpgradeProgress()
{
    qint64 size = QFileInfo(upgradeImageName).size();
    //TODO: update a progress bar
}

void HardDiskManager::testImage()
{
    if(!currentImage.exists())
        return;
#ifndef Q_OS_WIN32
    QString program = "qemu-img";
#elif defined(Q_OS_WIN32)
    QString program = "qemu/qemu-img.exe";
#endif
    QStringList arguments;
    QProcess *testProcess = new QProcess(this);
    arguments << "info" << currentImage.filePath();
    testProcess->start(program, arguments);
    testProcess->waitForFinished();
    //time to parse the output and get all the info available
    //splits are on colons.
    QStringList output = QString(testProcess->readAll()).split('\n');
    currentFormat =  output.at(1).split(':').at(1).simplified();
    emit imageFormat(currentFormat);
    if(currentFormat!="qcow2")
    {
        emit imageUpgradable(true);
        suspendable = false;
    }
    else
    {
        emit imageUpgradable(false);
       suspendable = true;
    }
    emit supportsSuspending(suspendable);
    QString virtSize = output.at(2).section('(', 1);
    virtSize.chop(6);
    virtualSize = virtSize.toInt();
    emit imageSize(virtualSize);
    emit phySize(currentImage.size());

    resumable = false;
    if(output.size()>6)
    {
        QString currentLine;
        QStringList list;
        for(int i = 7; i<output.size() - 1;i++)
        {
            currentLine = output.at(i);
            list = currentLine.split(QRegExp("\\W+"));
            if(currentLine.split(QRegExp("\\W+")).at(1) == "Default")
            {
                resumable = true;
            }
        }
    }
    emit supportsResuming(resumable);
}

bool HardDiskManager::event(QEvent * event)
{
    if(event->type() == QEvent::DynamicPropertyChange)
    {
        //any property changes dealt with in here
        QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
        if(propEvent->propertyName() == "hdd")
        {
            currentImage = QFileInfo(property("hdd").toString());
            testImage();
        }
        return false;
    }
    return QObject::event(event);
}


bool HardDiskManager::isSuspendable() const
{
    return suspendable;
}
