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



/*//TODO: the functionality in here really should be abstracted into another class, like MachineImage
void MachineTab::testHDDImage(const QString &path)
{
    QFileInfo *currentImage = new QFileInfo(path);
    
    if(currentImage->suffix()!="qcow")
    {
//        hddUpgradeButton->setHidden(false);
        suspendButton->setEnabled(false);
        resumeButton->setEnabled(false);
    }
    else
    {
//        hddUpgradeButton->setHidden(true);
        suspendButton->setEnabled(true);

        //test for a valid suspend/resume image
        QProcess *testImage = new QProcess();
        QStringList arguments;
        arguments <<"info" << path;
        testImage->start("qemu-img", arguments);
        testImage->waitForFinished();
        QString output = testImage->readAll();
        if(output.contains("Default"))
            resumeButton->setEnabled(true);
        else
            resumeButton->setEnabled(false);
    }
}
//TODO: the functionality in here really should be abstracted into another class, like MachineImage
void MachineTab::upgradeImage()
{
    if (QMessageBox::question(this, tr("Upgrade Confirmation"),
                              tr("This will upgrade your Hard Disk image to the qcow format.<br />This enables more advanced features such as suspend/resume on all operating systems and image compression on Windows.<br />Your old image will remain intact, so if you want to revert afterwards you may do so."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
      == QMessageBox::Yes)
    {
        QFileInfo *currentImage = new QFileInfo(hddPathLineEdit->text());
        QString finalName = currentImage->path() + '/' + currentImage->completeBaseName() + ".qcow";
        QStringList arguments;
        QProcess *upgradeProcess = new QProcess(this);
        connect(upgradeProcess, SIGNAL(started()), this, SLOT(upgradeImageStarted()));
        connect(upgradeProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(upgradeImageFinished(int)));
        
        arguments << "convert" << currentImage->filePath() << "-O" << "qcow2" << finalName;
        QString program = "qemu-img";
        upgradeProcess->start(program, arguments);
    }
}

//TODO: the functionality in here really should be abstracted into another class, like DiskImage
void MachineTab::upgradeImageStarted()
{
    startButton->setEnabled(false);
    resumeButton->setEnabled(false);
    
    hddUpgradeButton->setEnabled(false);
    hddPathLineEdit->setEnabled(false);
    hddUpgradeButton->setText(tr("Upgrading..."));
}
*/



void HardDiskManager::upgradeImage()
{
//assume image 0
    emit processingImage(true);

    QFileInfo currentImage = QFileInfo(property("hdd").toString());
    upgradeImageName = currentImage.path() + '/' + currentImage.completeBaseName() + ".qcow";
    QString program = "qemu-img";
    QStringList arguments;
    currentProcess = new QProcess(this);
    arguments << "info" << currentImage.filePath();
    currentProcess->start(program, arguments);
    connect(currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(getImageSize()));

    arguments.clear();
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

void HardDiskManager::getImageSize()
{
    QByteArray output = currentProcess->readAllStandardOutput();
    //TODO: parse out the file size, inerpret K, G, T? as multipliers...
    //save to oldSize
}
