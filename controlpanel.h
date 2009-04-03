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

/****************************************************************************
** C++ Interface: controlpanel
**
** Description: 
**
****************************************************************************/
#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "ui_controlpanel.h"
#include <QWidget>

class MachineTab;
class MachineConfigObject;
class MachineProcess;
class SettingsTab;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class ControlPanel : public QWidget , public Ui::ControlPanel
{
Q_OBJECT
public:
    explicit ControlPanel(MachineTab *parent);

    ~ControlPanel();

private:
    void makeConnections();
    void registerObjects();
    MachineTab *parent;
    MachineConfigObject *config;
private slots:
    void mediaActivate();
    void displayActivate();
    void usbActivate();
    void saveScreenshot();
    void running();
    void stopped();

    void optionChanged(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);

    void optAdded(const QString devName, const QString devPath);
    void optRemoved(const QString devName, const QString devPath);
};

#endif
