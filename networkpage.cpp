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

    //guest
    connect(addGuest, SIGNAL(clicked()), this, SLOT(addGuestInterface()));
    connect(delGuest, SIGNAL(clicked()), this, SLOT(delGuestInterface()));
    connect(guestView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(guestSelectionChanged(const QItemSelection &, const QItemSelection &)));

    //host
    connect(addHost, SIGNAL(clicked()), this, SLOT(addHostInterface()));
    connect(delHost, SIGNAL(clicked()), this, SLOT(delHostInterface()));
    connect(hostView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(hostSelectionChanged(const QItemSelection &, const QItemSelection &)));
}

void NetworkPage::setupModels()
{
    guestModel = new GuestInterfaceModel(config, this);
    guestView->setModel(guestModel);
    //guestView->hideColumn(3);
    //guestView->hideColumn(1);

    hostModel = new HostInterfaceModel(config, this);
    hostView->setModel(hostModel);
    //for(int i=2;i<hostModel->columnCount();i++)
    //    hostView->hideColumn(i);

}

void NetworkPage::registerObjects()
{
    config->registerObject(networkCheck, "network", QVariant(true));
    config->registerObject(networkEdit, "networkCustomOptions");
    config->registerObject(netAccelCheck, "netVirtio", QVariant(false));
}

void NetworkPage::addGuestInterface()
{
    guestModel->insertRows(guestModel->rowCount(), 1);
}

void NetworkPage::delGuestInterface()
{
    //qDebug("has selection? ");
    //qDebug( (guestView->selectionModel()->hasSelection()) ? "true":"false");
    if(guestView->selectionModel()->hasSelection())
        guestModel->removeRows(guestView->selectionModel()->selectedIndexes().first().row(), guestView->selectionModel()->selectedIndexes().size());
}

void NetworkPage::addHostInterface()
{
    hostModel->insertRows(hostModel->rowCount(), 1);
    hostTypeBox->setCurrentIndex(0);
}

void NetworkPage::delHostInterface()
{
    if(hostView->selectionModel()->hasSelection())
        hostModel->removeRows(hostView->selectionModel()->selectedIndexes().first().row(), hostView->selectionModel()->selectedIndexes().size());
}

void NetworkPage::guestSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    //disconnect other interfaces
    config->unregisterObject(nicModelCombo);
    config->unregisterObject(macEdit);
    config->unregisterObject(randomCheck);

    QString rowName = guestModel->rowName(selected.first().indexes().first().row());
    //show guest properties
    propertyStack->setCurrentIndex(5);
    //populate guest properties
    config->registerObject(nicModelCombo, "net-guest", rowName, "nic");
    config->registerObject(macEdit, "net-guest", rowName, "mac");
    config->registerObject(randomCheck, "net-guest", rowName, "randomize");
}

void NetworkPage::hostSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    //disconnect other interfaces
    config->unregisterObject(interfaceEdit);
    config->unregisterObject(interfaceEdit_2);
    config->unregisterObject(interfaceEdit_3);
    config->unregisterObject(bridgeEdit);
    config->unregisterObject(hardwareEdit);
    config->unregisterObject(hardwareEdit_2);
    config->unregisterObject(spanningCheck);
    config->unregisterObject(upScriptEdit);
    config->unregisterObject(downScriptEdit);
    config->unregisterObject(hostnameEdit);
    config->unregisterObject(tftpCheck);
    config->unregisterObject(tftpEdit);
    config->unregisterObject(bootpCheck);
    config->unregisterObject(bootpEdit);
    config->unregisterObject(udpButton);
    config->unregisterObject(tcpButton);
    config->unregisterObject(addressCombo);
    config->unregisterObject(portSpin);
    config->unregisterObject(hostTypeBox);

    QString rowName = hostModel->rowName(selected.first().indexes().first().row());

    config->registerObject(hostTypeBox, "net-host", rowName, "type", "User Mode");

    propertyStack->setCurrentIndex(hostTypeBox->currentIndex());

    config->registerObject(interfaceEdit, "net-host", rowName, "interface");
    config->registerObject(interfaceEdit_2, "net-host", rowName, "interface");
    config->registerObject(interfaceEdit_3, "net-host", rowName, "interface");
    config->registerObject(bridgeEdit, "net-host", rowName, "bridgeInterface");
    config->registerObject(hardwareEdit, "net-host", rowName, "hardwareInterface");
    config->registerObject(hardwareEdit_2, "net-host", rowName, "hardwareInterface");
    config->registerObject(spanningCheck, "net-host", rowName, "spanningTree");
    config->registerObject(upScriptEdit, "net-host", rowName, "ifUp");
    config->registerObject(downScriptEdit, "net-host", rowName, "ifDown");
    config->registerObject(hostnameEdit, "net-host", rowName, "hostname");
    config->registerObject(tftpCheck, "net-host", rowName, "tftp");
    config->registerObject(tftpEdit, "net-host", rowName, "tftpPath");
    config->registerObject(bootpCheck, "net-host", rowName, "bootp");
    config->registerObject(bootpEdit, "net-host", rowName, "bootpPath");
    config->registerObject(udpButton, "net-host", rowName, "vlanType");
    config->registerObject(tcpButton, "net-host", rowName, "vlanType");
    config->registerObject(addressCombo, "net-host", rowName, "address");
    config->registerObject(portSpin, "net-host", rowName, "port");

    

}












