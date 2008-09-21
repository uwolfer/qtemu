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

#ifndef HARDDISKMANAGER_H
#define HARDDISKMANAGER_H

#include <QObject>
#include <QEvent>
#include <QFileInfo>
#include "machinetab.h"


class QTimer;
class QProcess;
class MachineProcess;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class HardDiskManager : public QObject
{
Q_OBJECT
public:
    HardDiskManager(MachineProcess *parent = 0);

    ~HardDiskManager();


    bool imageIs();
    bool event(QEvent * event);
    bool isSuspendable() const;

private:
    void addDisk(const QString &path, const int address);
//data
    QStringList diskImages;


//used for some functions
    MachineProcess *parent;
    QString upgradeImageName;
    QTimer *updateProgressTimer;
    QProcess *currentProcess;
    QString currentFormat;
    QFileInfo currentImage;
    qint64 virtualSize;
    qint64 oldSize;
    bool suspendable;
    bool resumable;
public slots:
    void upgradeImage();
    void testImage();

private slots:
    void upgradeComplete(int status);
    void updateUpgradeProgress();

signals:
    void processingImage(bool processing);
    void imageUpgradable(bool elegability);
    void upgradeProgress(qint64 size, qint64 total);
    void error(const QString &errorString);
    void imageFormat(QString format);
    void imageSize(qint64 size);
    void phySize(qint64 size);
    void supportsSuspending(bool status);
    void supportsResuming(bool status);
};

#endif
