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

#include <QProcess>

class MachineProcess : public QProcess
{
    Q_OBJECT

public:
    MachineProcess(QObject *parent = 0);
    qint64 write(const QByteArray & byteArray);

public slots:
    void start();
    void resume(const QString& snapshotName);
    void resume();
    void suspend(const QString& snapshotName);
    void suspend();
    void stop();
    void forceStop();
    void togglePause();
    void name(const QString &name);
    void path(const QString &newPath);
    void cdRomPath(const QString &newPath);
    void floppyDiskPath(const QString &newPath);
    void smbFolderPath(const QString &newPath);
    void cdBoot(int value);
    void floppyBoot(int value);
    void snapshot(int value);
    void network(int value);
    void bridgedModeNetwork(int value);
    void localBridgeModeNetwork(int value);
    void sharedVlanNetwork(int value);
    void sound(int value);
    void soundSystem(int value);
    void memory(int value);
    void mouse(int value);
    void time(int value);
    void virtualization(int value);
    void cpu(int value);
    void useAdditionalOptions(int value);
    void networkCustomOptions(const QString& options);
    void additionalOptions(const QString& options);
    void changeCdrom();
    void changeFloppy();
signals:
    void suspending(const QString & snapshotName);
    void suspended(const QString & snapshotName);
    void resuming(const QString & snapshotName);
    void resumed(const QString & snapshotName);
    void error(const QString & errorText);
    void stdout(const QString & stdoutText);
    void stdin(const QString & stdoutText);

private:
    void getVersion();
    QString pathString;
    QString machineNameString;
    QString cdRomPathString;
    QString floppyDiskPathString;
    QString networkCustomOptionsString;
    QString additionalOptionsString;
    QString snapshotNameString;
    long versionMajor, versionMinor, versionBugfix, kvmVersion;
    bool bootFromCdEnabled;
    bool bootFromFloppyEnabled;
    bool snapshotEnabled;
    bool networkEnabled;
    bool soundEnabled;
    bool mouseEnabled;
    bool timeEnabled;
    bool virtualizationEnabled;
    bool additionalOptionsEnabled;
    bool paused;
    bool doResume;
    QByteArray useSoundSystem;
    int memoryInt;
    int cpuInt;

private slots:
    void afterExitExecute();
    void readProcess();
    void readProcessErrors();
    void writeDebugInfo(const QString& debugText);
    void resumeFinished(const QString& returnedText);
    void suspendFinished(const QString& returnedText);
};

#endif
