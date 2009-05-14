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


#ifndef NETWORKPAGE_H
#define NETWORKPAGE_H

#include <QWidget>

#include <QModelIndex>

#include "ui_networkpage.h"

class MachineConfigObject;
class GuestInterfaceModel;
class HostInterfaceModel;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/

/****************************************************************************
** C++ Interface: networkpage
**
** Description: 
**
****************************************************************************/
class NetworkPage : public QWidget , public Ui::NetworkPage
{
Q_OBJECT
public:
    explicit NetworkPage(MachineConfigObject *config, QWidget *parent = 0);

    ~NetworkPage();

private:
    void makeConnections();
    void setupPage();
    void setupModels();
    void registerObjects();
    MachineConfigObject *config;

    GuestInterfaceModel *guestModel;
    HostInterfaceModel *hostModel;
    bool changingSelection;

//dealing with model/view

private slots:
    void changeNetPage(bool state);

    void delGuestInterface();
    void addGuestInterface();

    void delHostInterface();
    void addHostInterface();

    void guestSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void hostSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

    void loadHostEthIfs();
};


//class hostInterfaceModel;

#endif
