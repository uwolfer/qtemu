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

#include "networkpage.h"
#include "machineconfigobject.h"
#include "interfacemodel.h"
/****************************************************************************
** C++ Implementation: networkpage
**
** Description: 
**
****************************************************************************/
NetworkPage::NetworkPage(MachineConfigObject *config, QWidget *parent)
 : QWidget(parent)
 , config(config)
{
    setupUi(this);
    setupModels();
    makeConnections();
    registerObjects();
}


NetworkPage::~NetworkPage()
{
}

void NetworkPage::changeNetPage(bool state)
{
    networkStack->setCurrentIndex((int)state);
}

void NetworkPage::makeConnections()
{
    connect(advancedButton, SIGNAL(toggled(bool)), this, SLOT(changeNetPage(bool)));

    connect(addGuest, SIGNAL(clicked()), this, SLOT(addGuestInterface()));
    connect(delGuest, SIGNAL(clicked()), this, SLOT(delGuestInterface()));
    
}

void NetworkPage::setupModels()
{
    guestModel = new GuestInterfaceModel(config, this);
    guestView->setModel(guestModel);
    guestView->hideColumn(3);
    guestView->hideColumn(1);
    //guestView->setRootIndex(guestModel);
}

void NetworkPage::registerObjects()
{
    config->registerObject(networkCheck, "network", QVariant(true));
    config->registerObject(networkEdit, "networkCustomOptions");
}

void NetworkPage::addGuestInterface()
{
    guestModel->insertRows(guestModel->rowCount(), 1);
}

void NetworkPage::delGuestInterface()
{
    if(guestView->selectionModel()->hasSelection())
        guestModel->removeRows(guestView->selectionModel()->selectedIndexes().first().row(), guestView->selectionModel()->selectedIndexes().size());
}












