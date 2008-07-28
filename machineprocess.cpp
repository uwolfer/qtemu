/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
** Copyright (C) 2008 Ben Klopfenstein <benklop @ gmail.com>
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

#include "machineprocess.h"
//#include "networksystem.h"
#include "config.h"

#include <QCoreApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTimer>

MachineProcess::MachineProcess(QObject *parent)
               :QProcess(parent),
                bootFromCdEnabled(false),
                bootFromFloppyEnabled(false),
                snapshotEnabled(false),
                networkEnabled(false),
                mouseEnabled(false),
                timeEnabled(false),
                additionalOptionsEnabled(false),
                memoryInt(-1),
                cpuInt(-1)
{
    paused=false;
    doResume=false;
    vncPort=0;
    soundSystem(0);
    getVersion();
    //networkSystem = new NetworkSystem(this);
    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcess()));
    connect(this, SIGNAL(readyReadStandardError()), this, SLOT(readProcessErrors()));
    connect(this, SIGNAL(stdout(const QString&)), this, SLOT(writeDebugInfo(const QString&)));
    connect(this, SIGNAL(stdin(const QString&)), this, SLOT(writeDebugInfo(const QString&)));
}


void MachineProcess::start()
{
    qDebug(property("nodeType").toByteArray());
    getVersion();
    //if(versionMajor == -1)//executable was not found

    QSettings settings("QtEmu", "QtEmu");
    QStringList env = QProcess::systemEnvironment();
    QStringList arguments;

    // Append the additional parameters first so wrapper programs like vde
    // can be used to start the virtual machine, e.g. vdeq kvm or vdeq qemu
    // where vdeq is specified as the command and kvm/qemu as the additional
    // option or parameter.
    if (additionalOptionsEnabled && !additionalOptionsString.isEmpty())
        arguments << additionalOptionsString.split(" ", QString::SkipEmptyParts);
    
    if (property("embeddedDisplay").toBool())
        arguments << "-vnc" << "localhost:" + property("vncPort").toString();

    if (networkEnabled)
    {   /*
        //use the new network setup if you are developing
        #ifdef DEVELOPER
        //load up network setup
        networkSystem->loadNics();
        if (networkSystem->numNics() != 0)
            arguments << networkSystem->getOptionList();
        #endif
        */
        if (!networkCustomOptionsString.isEmpty())
            arguments << networkCustomOptionsString.split(" ", QString::SkipEmptyParts);
        //#ifndef DEVELOPER
        else
            arguments << "-net" << "nic" << "-net" << "user";
        //#endif
    }
    else
        arguments << "-net" << "none";

    /*using the drive syntax, multiple drives can be added to a VM. drives can be specified as
     disconnected, allowing media to be inserted after bootup. the initialization can take the form
     of a loop, and the device settings can be put in an array of drive objects. this also would
     allow both IDE and SCSI drives to be specified, making more than 4 disks possible.
    in order to take advantage of this, the interface will need a custom drive widget that can be
     inserted multiple times easily.
     to save these complex settings, settings can be saved under <drives> with a specific <drive>
     in the .qte file. this does not work correctly with qemu < 0.9.1
   */
    if ((versionMajor >= 0 && versionMinor >= 9 && versionBugfix >= 1)|(kvmVersion>=60))
    {
        //TODO: modify to support multiple floppies and cdroms(index=0 and index=1)
        if (!cdRomPathString.isEmpty())
        {
        cdRomPathString = cdRomPathString.replace(QRegExp(","),",,");
        arguments << "-drive" << "file=" + cdRomPathString + ",if=ide,bus=1,unit=0,media=cdrom";
        //TODO:make the drive location configurable
        if (bootFromCdEnabled)
            arguments << "-boot" << "d";
        }
        else//allows the cdrom to exist if not specified
        {
        arguments << "-drive" << "if=ide,bus=1,unit=0,media=cdrom";
        //TODO:make the drive location configurable
        }

        if (!floppyDiskPathString.isEmpty())
        {
            arguments << "-drive" << "file=" + floppyDiskPathString + ",index=0,if=floppy";
            if (bootFromFloppyEnabled)
                arguments << "-boot" << "a";
        }
    }
    else //use old (<0.9.1) drive syntax, cdrom must exist on startup to be inserted
    {
        if (!cdRomPathString.isEmpty())
        {
            arguments << "-cdrom" << cdRomPathString;
            if (bootFromCdEnabled)
                arguments << "-boot" << "d";
        }
        if (!floppyDiskPathString.isEmpty())
        {
          arguments << "-fda" << floppyDiskPathString;
          if (bootFromFloppyEnabled)
              arguments << "-boot" << "a";
        }
    }

    
    if (soundEnabled)
    {
    //FIXME: this does not work yet with alsa... no idea why. specifying oss, which is the default anyway, works ok.
    // other possible values are "wav", "none", "sdl", and maybe "esd"...
    //we can run qemu -audio-help and any line in the format "Name: <value>" will list a driver we can use.
    //TODO:on windows and mac i assume we have to stick with the default...
        arguments << "-soundhw" << "es1370";
        env << "QEMU_AUDIO_DRV=" + useSoundSystem;
    }
    
    if (memoryInt > 0)
      arguments << "-m" << QString::number(memoryInt);

    if (cpuInt > 1)
      arguments << "-smp" << QString::number(cpuInt);

    if (mouseEnabled)
        arguments << "-usb" << "-usbdevice" << "tablet";

    if (timeEnabled)
        arguments << "-localtime";

    if (!virtualizationEnabled && kvmVersion > 0)
        arguments << "-no-kvm";
    else if (!virtualizationEnabled)
        arguments << "-no-kqemu";
    else if (virtualizationEnabled && kvmVersion <= 0)
        arguments << "-kernel-kqemu";

    if (doResume)
        arguments << "-loadvm" << snapshotNameString;

    //allow access to the monitor via stdio
    //FIXME: does this work in windows? if so, then most of the new features will work in
    // windows too. if not, the pipe:filename option might work, if we can make a named pipe...
    // http://en.wikipedia.org/wiki/Named_pipe
#ifndef Q_OS_WIN32
    arguments << "-monitor" << "stdio";
#endif

    if((versionMajor >= 0 && versionMinor >= 9 && versionBugfix >= 1)|(kvmVersion>=60))
        arguments << "-name" << "\"" + machineNameString + "\"";

    // Add the VM image name
    // And use the temp file if snapshot is enabled
    if (snapshotEnabled)
    {
        createTmp();
        arguments << pathString + ".tmp";
    }
    else 
        arguments << pathString;

#ifdef DEVELOPER
    QString debugString = QString();
    for (int i = 0; i < arguments.size(); ++i)
        debugString = debugString + arguments.at(i).toLocal8Bit().constData() + ' ';
    qDebug(debugString.toLocal8Bit().constData());
#endif
    connect(this, SIGNAL(stdout(const QString&)), this, SLOT(startedBooting(const QString&)));
    connect(this, SIGNAL(finished(int)), SLOT(afterExitExecute()));

    QString command = settings.value("beforeStart").toString();
    if (!command.isEmpty())
    {
        QStringList commandList;
        commandList = command.split("\n");
        for (int i = 0; i < commandList.size(); ++i)
        {
            QProcess::start(commandList.at(i).toLocal8Bit().constData());
            while (waitForFinished())
            {
                QTime sleepTime = QTime::currentTime().addMSecs(5);
                while (QTime::currentTime() < sleepTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
        }
    }

    setEnvironment(env);
    //#ifdef DEVELOPER
    //qDebug("Environment:");
    //qDebug(env.join("\n").toAscii());
    //#endif
    
#ifndef Q_OS_WIN32
    QProcess::start(settings.value("command", "qemu").toString(), arguments);
#elif defined(Q_OS_WIN32)
    arguments << "-L" << ".";
    QString qemuCommand = settings.value("command", QCoreApplication::applicationDirPath() + "/qemu/qemu.exe").toString();
    QDir *path = new QDir(qemuCommand);
    path->cdUp();
    setWorkingDirectory(path->path());
    start(qemuCommand, arguments);
#endif
}

void MachineProcess::afterExitExecute()
{
    if(snapshotEnabled)
        deleteTmp(0);
    else if(QFile::exists(pathString + ".tmp"))
        commitTmp();

    QSettings settings("QtEmu", "QtEmu");

    QString command = settings.value("afterExit").toString();
    if (!command.isEmpty())
    {
        QStringList commandList;
        commandList = command.split("\n");
        for (int i = 0; i < commandList.size(); ++i)
        {
            QProcess::start(commandList.at(i).toLocal8Bit().constData());
            while (waitForFinished())
            {
                QTime sleepTime = QTime::currentTime().addMSecs(5);
                while (QTime::currentTime() < sleepTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
        }
    }

    doResume=false;
    //networkSystem->clearAllNics();
}

void MachineProcess::path(const QString &newPath)
{
    pathString = newPath;
}

void MachineProcess::cdRomPath(const QString &newPath)
{
    cdRomPathString = newPath;
}

void MachineProcess::cdBoot(int value)
{
    bootFromCdEnabled = (value == Qt::Checked);
}

void MachineProcess::floppyDiskPath(const QString &newPath)
{
    floppyDiskPathString = newPath;
}

void MachineProcess::floppyBoot(int value)
{
    bootFromFloppyEnabled = (value == Qt::Checked);
}

void MachineProcess::snapshot(int value)
{
    snapshotEnabled = (value == Qt::Checked);
}

void MachineProcess::network(int value)
{
    networkEnabled = (value == Qt::Checked);
}

void MachineProcess::sound(int value)
{
    soundEnabled = (value == Qt::Checked);
}

void MachineProcess::time(int value)
{
    timeEnabled = (value == Qt::Checked);
}

void MachineProcess::virtualization(int value)
{
    virtualizationEnabled = (value == Qt::Checked);
}

void MachineProcess::mouse(int value)
{
    mouseEnabled = (value == Qt::Checked);
}

void MachineProcess::useAdditionalOptions(int value)
{
    additionalOptionsEnabled = (value == Qt::Checked);
}

void MachineProcess::useVnc(int port)
{
        vncPort = port;
}

void MachineProcess::memory(int value)
{
    memoryInt = value;
}

void MachineProcess::cpu(int value)
{
    cpuInt = value;
}

void MachineProcess::networkCustomOptions(const QString& options)
{
    networkCustomOptionsString = options;
}

void MachineProcess::additionalOptions(const QString& options)
{
    additionalOptionsString = options;
}

void MachineProcess::name(const QString & name)
{
    machineNameString = name;
}

void MachineProcess::resume() {resume("Default");}
void MachineProcess::resume(const QString & snapshotName)
{
    snapshotNameString = snapshotName;
    if(state()==QProcess::Running)
    {
        write("loadvm " + snapshotName.toAscii() + '\n');
        emit resuming(snapshotName);
    }
    else
    {
        doResume=true;
        start();
        emit resuming(snapshotName);
        write("\n");
    }
    connect(this, SIGNAL(stdout(const QString&)),this,SLOT(resumeFinished(const QString&)));

}

void MachineProcess::resumeFinished(const QString& returnedText)
{
    if(returnedText.contains("(qemu)"))
    {
        emit resumed(snapshotNameString);
        disconnect(this, SIGNAL(stdout(const QString&)),this,SLOT(resumeFinished(const QString&)));
    }
    //might need to reconnect the usb tablet here...
}

void MachineProcess::suspend() {suspend("Default");}
void MachineProcess::suspend(const QString & snapshotName)
{
    snapshotNameString = snapshotName;
    emit suspending(snapshotName);
    //usb is not properly resumed, so we need to disable it first in order to keep things working afterwords.
    //this also means that we need to dynamically get usb devices to unload and save them with the qtemu config
    //file for proper usb support with suspend. as it is we just unload the tablet, which is all we know about.
    if(mouseEnabled) 
    {
        write("usb_del 0.1\n");
        sleep(2);//wait for the guest OS to notice
    }
    write("stop\n");
    write("savevm " + snapshotName.toAscii() + '\n');
    connect(this, SIGNAL(stdout(const QString&)),this,SLOT(suspendFinished(const QString&)));
}

void MachineProcess::suspendFinished(const QString& returnedText)
{
    if(returnedText == "(qemu)")
    {
        write("cont\n");
        emit suspended(snapshotNameString);
        disconnect(this, SIGNAL(stdout(const QString&)),this,SLOT(suspendFinished(const QString&)));
    }
}

void MachineProcess::togglePause()
{
     paused ? write("cont\n") : write("stop\n");
     paused = !paused;
}

void MachineProcess::stop()
{
    write("system_powerdown\n");
}

void MachineProcess::forceStop()
{
    write("quit\n");
}

void MachineProcess::readProcess()
{
    QByteArray rawOutput = readAllStandardOutput();
    emit rawConsole(rawOutput); //for connection to a fully interactive console... eventually
    QString convOutput = rawOutput;
    QStringList splitOutput = convOutput.split("[K");
    if (splitOutput.last()==splitOutput.first())
    {
        emit cleanConsole(convOutput);
        emit stdout(convOutput.simplified());
    }
    else 
    {
        if(!splitOutput.last().isEmpty())
        {
            QString cleanOutput = splitOutput.last().remove(QRegExp("\[[KD]."));
            emit cleanConsole(cleanOutput);
            emit stdout(cleanOutput.simplified());
        }
    }
}

void MachineProcess::readProcessErrors()
{
    QString errorText = readAllStandardError();
    if(!supressedErrors.isEmpty())
        for(int i=0;i<supressedErrors.size();i++)
            if(errorText.contains(supressedErrors.at(i)))
                return;
    emit error(errorText);
}

qint64 MachineProcess::write ( const QByteArray & byteArray )
{
    emit stdin(((QString)byteArray).simplified());
    emit cleanConsole((QString)byteArray);
    return QProcess::write(byteArray);
}

void MachineProcess::writeDebugInfo(const QString & debugText)
{
#ifdef DEVELOPER
    qDebug(debugText.toAscii());
#endif
}

void MachineProcess::getVersion()
{
    QSettings settings("QtEmu", "QtEmu");
    QString versionString;
    QProcess *findVersion = new QProcess(this);

#ifndef Q_OS_WIN32
    QString qemuCommand = settings.value("command", "qemu").toString();
#elif defined(Q_OS_WIN32)
    QString qemuCommand = settings.value("command", QCoreApplication::applicationDirPath() + "/qemu/qemu.exe").toString();
    QDir *path = new QDir(qemuCommand);
    path->cdUp();
    setWorkingDirectory(path->path());
#endif

    findVersion->start(qemuCommand);
    findVersion->waitForFinished();

    if(findVersion->error() == QProcess::FailedToStart)
    {
        versionMajor = -1;
        versionMinor = -1;
        versionBugfix = -1;
        kvmVersion = -1;
        emit error(tr("Either the qemu binary does not exist, or it is not executable at ") + qemuCommand);
        return;
    }
    
    QString infoString = findVersion->readLine();
    QStringList infoStringList = infoString.split(" ");
    
    versionString = infoStringList.at(4);
    QStringList versionStringList = versionString.split(".");
    versionMajor = versionStringList.at(0).toInt();
    versionMinor = versionStringList.at(1).toInt();
    versionBugfix = versionStringList.at(2).toInt();
    versionString = infoStringList.at(5);
    versionString.remove(QRegExp("[(),]"));
    if(versionString.contains(QRegExp("kvm")))
    {
        kvmVersion = versionString.remove(QRegExp("kvm-")).toInt();
    }
    else
        kvmVersion = 0;

    #ifdef DEVELOPER
    qDebug(("kvm: " + QString::number(kvmVersion) + " qemu: " + QString::number(versionMajor) + '.' + QString::number(versionMinor) + '.' + QString::number(versionBugfix)).toAscii());
    #endif
}

void MachineProcess::soundSystem(QString systemName)
{
    useSoundSystem = systemName.toAscii();
}

void MachineProcess::changeCdrom()
{
    //handle differing version syntax...
    if ((versionMajor >= 0 && versionMinor >= 9 && versionBugfix >= 1)|(kvmVersion>=60))
        write("eject -f ide1-cd0\n");
    else
        write("eject -f cdrom\n");
    QTimer::singleShot(5000, this, SLOT(loadCdrom()));
}

//TODO: accept a drive assignment to eject/insert.
void MachineProcess::changeFloppy()
{
    //handle differing version syntax...
    //if ((versionMajor >= 0 && versionMinor >= 9 && versionBugfix >= 1)|(kvmVersion>=60))
    write("eject -f floppy\n");//might need to be fda , not floppy
    write("change floppy " + floppyDiskPathString.toAscii() + '\n');
}

//TODO:a wizard needs to be made to set up sudo to work without manual intervention.
void MachineProcess::smbFolderPath(const QString & newPath)
{
    qDebug("feature temporarily disabled");//networkSystem->addSambaDir(newPath);
}

void MachineProcess::supressError(QString errorText)
{
    supressedErrors << errorText;
}

void MachineProcess::startedBooting(const QString& text)
{
    if(text.contains("(qemu)"))
    {
        #ifdef DEVELOPER
        qDebug("booting started");
        #endif
        emit booting();
        disconnect(this, SIGNAL(stdout(const QString&)), this, SLOT(startedBooting(const QString&)));
    }
}

void MachineProcess::loadCdrom()
{
    //handle differing version syntax...
    if ((versionMajor >= 0 && versionMinor >= 9 && versionBugfix >= 1)|(kvmVersion>=60))
        write("change ide1-cd0 " + cdRomPathString.toAscii() + '\n');
    else
        write("change cdrom" + cdRomPathString.toAscii() + '\n'); 
}

void MachineProcess::commitTmp()
{
    QProcess *commitTmpProcess = new QProcess(this);
    commitTmpProcess->start("qemu-img", QStringList() << "commit" << pathString + ".tmp");
    connect(commitTmpProcess, SIGNAL(finished (int, QProcess::ExitStatus)), this, SLOT(deleteTmp(int)));
}

void MachineProcess::deleteTmp(int successfulCommit)
{
 if(successfulCommit == 0)
    QFile::remove( pathString + ".tmp" );
}

void MachineProcess::createTmp()
{
    if(QFile::exists(pathString + ".tmp"))
        return;

    QProcess *createTmpProcess = new QProcess(this);
    createTmpProcess->start("qemu-img", QStringList() << "create" << "-f" << "qcow2" << "-b" << pathString << pathString + ".tmp");
    createTmpProcess->waitForFinished();
}
