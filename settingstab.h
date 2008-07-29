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
**
** C++ Interface: settingstab
**
** Description: 
**
****************************************************************************/

#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include "ui_settingstab.h"
#include <QFrame>

class MachineConfigObject;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class SettingsTab : public QFrame, public Ui::SettingsTab
{
Q_OBJECT
public:
    SettingsTab(MachineConfigObject *config, QWidget *parent = 0);

    ~SettingsTab();

private:
MachineConfigObject *config;

void registerWidgets();
void setIcons();
void setupHelp();
private slots:

void changeHelpTopic(int page);

};

#endif
