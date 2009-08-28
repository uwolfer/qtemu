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

#ifndef MACHINEPROCESS_H
#define MACHINEPROCESS_H

#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include "qtemuenvironment.h"
#include "harddiskmanager.h"

class NetConfig;
class UsbConfig;

class MachineProcess : public QObject
{
    Q_OBJECT

public:
    enum ProcessState {NotRunning = 0, Starting = 1, Running = 2, Stopping = 3, Saving = 4};

    MachineProcess(MachineTab *parent = 0);
    qint64 write(const QByteArray & byteArray);

    HardDiskManager* getHdManager();
    UsbConfig* getUsbConfig();
    QProcess* getProcess();
    bool event(QEvent *event);
    MachineProcess::ProcessState state();
    
public slots:
    void start();
    void resume(const QString& snapshotName = QString("Default"));
    void suspend(const QString& snapshotName = QString("Default"));
    void stop();
    void forceStop();
    void togglePause();
    void changeCdrom();
    void changeFloppy();
    void loadCdrom();
    void connectIfRunning();

signals:
    void suspending(const QString & snapshotName);
    void suspended(const QString & snapshotName);
    void booting();
    void resuming(const QString & snapshotName);
    void resumed(const QString & snapshotName);
    void error(const QString & errorText);
    void stdout(const QString & stdoutText);
    void stdin(const QString & stdoutText);
    void rawConsole(const QString & consoleOutput);
    void cleanConsole(const QString & consoleOutput);
    void stateChanged(MachineProcess::ProcessState newState);
    void started();
    void finished();
    
private:
    void getVersion();
    void commitTmp();
    void createTmp();
    bool checkIfRunning();
    void connectToProcess();
    QStringList buildParamList();
    QStringList buildEnvironment();

    QString snapshotNameString;
    QtEmuEnvironment env;
    long versionMajor, versionMinor, versionBugfix, kvmVersion;
    bool paused;
    bool doResume;
    HardDiskManager *hdManager;
    QString lastOutput;
    QStringList outputParts;
    MachineProcess::ProcessState myState;
    NetConfig *netConfig;
    UsbConfig *usbConfig;

    QLocalSocket *console;
    QProcess *process;

private slots:
    void beforeRunExecute();
    void afterExitExecute();
    void readProcess();
    void readProcessErrors();
    void writeDebugInfo(const QString& debugText);
    void resumeFinished(const QString& returnedText);
    void suspendFinished(const QString& returnedText);
    void deleteTmp(int successfulCommit);
    void saveState(MachineProcess::ProcessState newState);
};

#endif
