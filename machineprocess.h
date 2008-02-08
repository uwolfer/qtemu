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

#ifndef MACHINEPROCESS_H
#define MACHINEPROCESS_H

#include <QProcess>

class MachineProcess : public QProcess
{
    Q_OBJECT

public:
    MachineProcess(QObject *parent = 0);

public slots:
    void start();
    void path(const QString &newPath);
    void cdRomPath(const QString &newPath);
    void floppyDiskPath(const QString &newPath);
    void cdBoot(int value);
    void floppyBoot(int value);
    void snapshot(int value);
    void network(int value);
    void sound(int value);
    void memory(int value);
    void mouse(int value);
    void time(int value);
    void cpu(int value);
    void useAdditionalOptions(int value);
    void networkCustomOptions(const QString& options);
    void additionalOptions(const QString& options);

private:
    QString pathString;
    QString cdRomPathString;
    QString floppyDiskPathString;
    QString networkCustomOptionsString;
    QString additionalOptionsString;
    bool bootFromCdEnabled;
    bool bootFromFloppyEnabled;
    bool snapshotEnabled;
    bool networkEnabled;
    bool soundEnabled;
    bool mouseEnabled;
    bool timeEnabled;
    bool additionalOptionsEnabled;
    int memoryInt;
    int cpuInt;

private slots:
    void afterExitExecute();
};

#endif
