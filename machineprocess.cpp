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
#include "netconfig.h"
#include "usbconfig.h"
#include "config.h"

#include <QCoreApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTimer>

MachineProcess::MachineProcess(MachineTab *parent)
  : QProcess(parent)
    , paused(false)
    , doResume(false)
    , hdManager(new HardDiskManager(this))
{
    changeState(QProcess::state());
    getVersion();
    netConfig = new NetConfig(this, parent->machineConfigObject);
    usbConfig = new UsbConfig(this, parent->machineConfigObject);

    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcess()));
    connect(this, SIGNAL(readyReadStandardError()), this, SLOT(readProcessErrors()));
    connect(this, SIGNAL(stdout(const QString&)), this, SLOT(writeDebugInfo(const QString&)));
    connect(this, SIGNAL(stdin(const QString&)), this, SLOT(writeDebugInfo(const QString&)));

    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(changeState(QProcess::ProcessState)));
}

HardDiskManager * MachineProcess::getHdManager()
{
    return hdManager;
}

UsbConfig* MachineProcess::getUsbConfig()
{
    return usbConfig;
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
    if (property("useAdditionalOptions").toBool() && !property("additionalOptions").toString().isEmpty())
        arguments << property("additionalOptions").toString().split(' ', QString::SkipEmptyParts);
    
    if (property("embeddedDisplay").toBool())
    {
        if(property("vncTransport").toString() == "tcp")
            arguments << "-vnc" << property("vncHost").toString() + ':' + property("vncPort").toString();
        else
        {
            QString socketLocation = property("hdImage").toString();
            socketLocation = socketLocation.replace(QRegExp("[.][^.]+$"), ".vnc");
            arguments << "-vnc" << "unix:" + socketLocation;
        }
    }

    if(property("hiRes").toBool())
    {
    	if(kvmVersion >= 78)
    		arguments << "-vga" << "std"; //TODO: other options are cirrus and vmware.. maybe we can make use of this.
    	else
    		arguments << "-std-vga";
   	}

    if(!property("acpi").toBool())
        arguments << "-no-acpi";

    if (property("network").toBool())
    {
        if (!property("networkCustomOptions").toString().isEmpty())
            arguments << property("networkCustomOptions").toString().split(' ', QString::SkipEmptyParts);
        else
        {
            arguments << netConfig->getOptionString();
        }
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
        if (!property("cdrom").toString().isEmpty())
        {
        QString cdRomPathString = property("cdrom").toString().replace(QRegExp(","),",,");
        arguments << "-drive" << "file=" + cdRomPathString + ",if=ide,bus=1,unit=0,media=cdrom";
        //TODO:make the drive location configurable
        if (property("bootFromCd").toBool())
            arguments << "-boot" << "d";
        }
        else//allows the cdrom to exist if not specified
        {
        arguments << "-drive" << "if=ide,bus=1,unit=0,media=cdrom";
        //TODO:make the drive location configurable
        }

        if (!property("floppy").toString().isEmpty())
        {
            arguments << "-drive" << "file=" + property("floppy").toString() + ",index=0,if=floppy";
            if (property("bootFromFloppy").toBool())
                arguments << "-boot" << "a";
        }
    }
    else //use old (<0.9.1) drive syntax, cdrom must exist on startup to be inserted
    {
        if (!property("cdrom").toString().isEmpty())
        {
            arguments << "-cdrom" << property("cdrom").toString();
            if (property("bootFromCd").toBool())
                arguments << "-boot" << "d";
        }
        if (!property("floppy").toString().isEmpty())
        {
          arguments << "-fda" << property("floppy").toString();
          if (property("bootFromFloppy").toBool())
              arguments << "-boot" << "a";
        }
    }

    
    if (property("sound").toBool())
    {
    //TODO:on windows and mac i assume we have to stick with the default... (directSound / coreAudio) .. need to ifndef this for those platforms.
    //TODO: allow selecting hardware
        arguments << "-soundhw" << "es1370";
        QString driver = "oss";
        if(property("soundSystem").toString() == tr("ALSA"))
            driver = "alsa";
        else if(property("soundSystem").toString() == tr("OSS"))
            driver = "oss";
        else if(property("soundSystem").toString() == tr("PulseAudio"))
            driver = "pa";
        else if(property("soundSystem").toString() == tr("ESD"))
            driver = "esd";
        else
            driver = property("soundSystem").toString();

        env << "QEMU_AUDIO_DRV=" + driver;
    }
    
    if (property("memory").toInt() > 0)
      arguments << "-m" << QString::number(property("memory").toInt());

    if (property("cpu").toInt() > 1)
      arguments << "-smp" << QString::number(property("cpu").toInt());

    if (property("usbSupport").toBool())
    {
        arguments << "-usb";
        arguments << usbConfig->getOptionString();
    }

    if (property("usbSupport").toBool() && property("mouse").toBool())
        arguments << "-usbdevice" << "tablet";

    if (property("time").toBool())
        arguments << "-localtime";

    if (!(property("virtualization").toBool()) && kvmVersion > 0)
        arguments << "-no-kvm";
    else if (!(property("virtualization").toBool()))
        arguments << "-no-kqemu";
    else if (property("virtualization").toBool() && kvmVersion <= 0)
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
        arguments << "-name" << "\"" + property("name").toString() + "\"";

    // Add the VM image name
    arguments << "-drive";
    // And use the temp file if snapshot is enabled
    if (property("snapshot").toBool())
    {
        createTmp();
        //arguments << pathString + ".tmp";
        arguments << "file=" + property("hdd").toString() + ".tmp" + (property("hddVirtio").toBool()?",if=virtio":"");
    }
    else 
        //arguments << pathString;
        arguments << "file=" + property("hdd").toString() + + (property("hddVirtio").toBool()?",if=virtio":"");

#ifdef DEVELOPER
    QString debugString;
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
        commandList = command.split('\n');
        QStringList paramList;
        for (int i = 0; i < commandList.size(); ++i)
        {
            paramList = commandList.at(i).split(' ');//FIXME: this will split parameters even if the space is enclosed in quotes or escaped. this is not quite right. same below
            if(paramList.size()==1)
                QProcess::start(commandList.at(i).toLocal8Bit().constData());
            else
                QProcess::start(paramList.takeFirst().toLocal8Bit().constData(),paramList);
            while (waitForFinished())
            {
                QTime sleepTime = QTime::currentTime().addMSecs(5);
                while (QTime::currentTime() < sleepTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
        }
    }
    command = property("execBefore").toString();
    if (property("enableExecBefore").toBool()&&!command.isEmpty())
    {
        QStringList commandList;
        commandList = command.split('\n');
        QStringList paramList;
        for (int i = 0; i < commandList.size(); ++i)
        {
            paramList = commandList.at(i).split(' ');//FIXME
            if(paramList.size()==1)
                QProcess::start(commandList.at(i).toLocal8Bit().constData());
            else
                QProcess::start(paramList.takeFirst().toLocal8Bit().constData(),paramList);
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
    QDir path(qemuCommand);
    path.cdUp();
    setWorkingDirectory(path.path());
    start(qemuCommand, arguments);
#endif

    emit cleanConsole(settings.value("command", "qemu").toString() + " " + arguments.join(" "));

}

void MachineProcess::afterExitExecute()
{
    if(property("snapshot").toBool())
        deleteTmp(0);
    else if(QFile::exists(property("hdd").toString() + ".tmp"))
        commitTmp();

    QSettings settings("QtEmu", "QtEmu");

    QString command = settings.value("afterExit").toString();
    if (!command.isEmpty())
    {
        QStringList commandList;
        commandList = command.split('\n');
        QStringList paramList;
        for (int i = 0; i < commandList.size(); ++i)
        {
            paramList = commandList.at(i).split(' ');//FIXME
            if(paramList.size()==1)
                QProcess::start(commandList.at(i).toLocal8Bit().constData());
            else
                QProcess::start(paramList.takeFirst().toLocal8Bit().constData(),paramList);
            while (waitForFinished())
            {
                QTime sleepTime = QTime::currentTime().addMSecs(5);
                while (QTime::currentTime() < sleepTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
        }
    }
    command = property("execAfter").toString();
    if (property("enableExecAfter").toBool()&&!command.isEmpty())
    {
        QStringList commandList;
        commandList = command.split('\n');
        QStringList paramList;
        for (int i = 0; i < commandList.size(); ++i)
        {
            paramList = commandList.at(i).split(' ');//FIXME
            if(paramList.size()==1)
                QProcess::start(commandList.at(i).toLocal8Bit().constData());
            else
                QProcess::start(paramList.takeFirst().toLocal8Bit().constData(),paramList);
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

void MachineProcess::resume() {resume("Default");}
void MachineProcess::resume(const QString & snapshotName)
{
    snapshotNameString = snapshotName;
    if(state()==MachineProcess::Running)
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
    if(property("mouse").toBool()) 
    {
        write("usb_del 0.1\n");
        sleep(2);//wait for the guest OS to notice
    }
    write("stop\n");
    write("savevm " + snapshotName.toAscii() + '\n');
    changeState(MachineProcess::Saving);
    connect(this, SIGNAL(stdout(const QString&)),this,SLOT(suspendFinished(const QString&)));
}

void MachineProcess::suspendFinished(const QString& returnedText)
{
    if(returnedText == "(qemu)")
    {
        write("cont\n");
        emit suspended(snapshotNameString);
        changeState(MachineProcess::NotRunning);
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
        emit cleanConsole(convOutput.trimmed());
        emit stdout(convOutput.simplified());
        lastOutput.append(convOutput.simplified());
    }
    else 
    {
        if(!splitOutput.last().isEmpty())
        {
            QString cleanOutput = splitOutput.last().remove(QRegExp("\[[KD]."));
            emit cleanConsole(cleanOutput.trimmed());
            emit stdout(cleanOutput.simplified());
            lastOutput.append(convOutput.simplified());
        }
    }
    outputParts = lastOutput.split("(qemu)");
    //qDebug(outputParts.last().toAscii());
}

void MachineProcess::readProcessErrors()
{
    QString errorText = readAllStandardError();
    emit error(errorText);
}

qint64 MachineProcess::write ( const QByteArray & byteArray )
{
    emit stdin(((QString)byteArray).simplified());
    emit cleanConsole(((QString)byteArray).trimmed());
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
 int *qemuVersion = env.getQemuVersion();
 versionMajor = qemuVersion[0];
 versionMinor = qemuVersion[1];
 versionBugfix = qemuVersion[2];
 kvmVersion = env.getKvmVersion();
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
    write("change floppy " + property("floppy").toByteArray() + '\n');
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
        write("change ide1-cd0 " + property("cdrom").toByteArray() + '\n');
    else
        write("change cdrom" + property("cdrom").toByteArray() + '\n'); 
}

void MachineProcess::commitTmp()
{
    changeState(MachineProcess::Saving);
    QProcess commitTmpProcess;
    commitTmpProcess.start("qemu-img", QStringList() << "commit" << property("hdd").toString() + ".tmp");
    connect(&commitTmpProcess, SIGNAL(finished (int, QProcess::ExitStatus)), this, SLOT(deleteTmp(int)));
}

void MachineProcess::deleteTmp(int successfulCommit)
{
    if(successfulCommit == 0)
        QFile::remove( property("hdd").toString() + ".tmp" );
    changeState(MachineProcess::NotRunning);
}

void MachineProcess::createTmp()
{
	//this mkaes it so if the temp file exists we use it... this could result in all sorts of wierd behavior.
    //if(QFile::exists(property("hdd").toString() + ".tmp"))
        //return;

    QProcess createTmpProcess;
    createTmpProcess.start("qemu-img", QStringList() << "create" << "-f" << "qcow2" << "-b" << property("hdd").toString() << property("hdd").toString() + ".tmp");
    createTmpProcess.waitForFinished();
}

bool MachineProcess::event(QEvent * event)
{
    if(event->type() == QEvent::DynamicPropertyChange)
    {
        //any property changes dealt with in here
        QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
        if(propEvent->propertyName() == "mouse")
        {
            if(!property("mouse").toBool())
            {
                //TODO: we need to actually detect the proper device...
                write("usb_del 0.1\n");
            }
            else
            {
                write("usb_add tablet\n");
            }
        }
        return false;
    }
    return QProcess::event(event);
}

void MachineProcess::changeState(MachineProcess::ProcessState newState)
{
    myState = newState;
    emit stateChanged(newState);
}

void MachineProcess::changeState(QProcess::ProcessState newState)
{

    changeState((MachineProcess::ProcessState)newState);
}

MachineProcess::ProcessState MachineProcess::state()
{
    return myState;
}
