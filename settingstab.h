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

#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "ui_settingstab.h"
#include "machineprocess.h"
#include "machinetab.h"
#include <QFrame>

class MachineConfigObject;
class NetworkPage;
class UsbPage;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class SettingsTab : public QFrame, public Ui::SettingsTab
{
Q_OBJECT
public:
    explicit SettingsTab(MachineConfigObject *config, MachineTab *parent = 0);

    ~SettingsTab();

    UsbPage *getUsbPage();

public slots:
    void setNewCdImagePath();
    void setNewFloppyImagePath();
    void setVirtSize(qint64 size);
    void setPhySize(qint64 size);

private:
    MachineConfigObject *config;
    MachineTab *parent;
    QString myMachinesPath;
    NetworkPage *netPage;
    UsbPage *usbPageWidget;
    void registerWidgets();
    void setupHelp();
    void setupConnections();
    void getSettings();
    void getDrives();
    void disableUnsupportedOptions();

private slots:

    void changeHelpTopic();


    //file select dialogs
    void setNewHddPath();

    //warning dialogs
    void confirmUpgrade();

    void optAdded(const QString devName, const QString devPath);
    void optRemoved(const QString devName, const QString devPath);

signals:
    void upgradeHdd();
};

#endif
