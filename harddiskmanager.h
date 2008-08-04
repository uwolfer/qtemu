//
// C++ Interface: harddiskmanager
//
// Description: 
//
//
// Author: Ben Klopfenstein <benklop@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HARDDISKMANAGER_H
#define HARDDISKMANAGER_H

#include <QObject>
#include "machinetab.h"

class QTimer;
class QProcess;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class HardDiskManager : public QObject
{
Q_OBJECT
public:
    HardDiskManager(QObject *parent = 0);

    ~HardDiskManager();


    void testHDDImage(const QString &path);

    void upgradeImageStarted();
    void upgradeImageFinished(const int &exitCode);
public slots:
    void upgradeImage();

private:
    void addDisk(const QString &path, const int address);
    QString upgradeImageName;
    QTimer *updateProgressTimer;
    QProcess *currentProcess;

    qint64 oldSize;

private slots:
    void upgradeComplete(int status);
    void updateUpgradeProgress();
    void getImageSize();

signals:
    void processingImage(bool processing);
    void imageUpgradable(bool elegability);
    void upgradeProgress(qint64 size, qint64 total);
    void error(const QString &errorString);
};

#endif
