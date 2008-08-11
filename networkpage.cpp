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


/****************************************************************************
** C++ Implementation: GuestInterfaceModel
**
** Description: a model for guest interfaces
**
****************************************************************************/
GuestInterfaceModel::GuestInterfaceModel(MachineConfigObject * config, QString nodeType, QObject * parent)
  : QAbstractTableModel(parent)
  , config(config)
  , nodeType(nodeType)
{

}

//display the model

int GuestInterfaceModel::rowCount(const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;
    int rows = config->getConfig()->getNumOptions(nodeType, "");
    qDebug("rows %i", rows);
    return rows;
}

int GuestInterfaceModel::columnCount(const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;
    int cols = config->getConfig()->getNumOptions(nodeType, "*");
    qDebug("cols %i", cols);
    return cols;
}

QVariant GuestInterfaceModel::data(const QModelIndex & index, int role) const
{
    //maybe this could be optimized by caching the stringlists...
    QString guestInterface = config->getConfig()->getAllOptionNames(nodeType, "").at(index.row());
    QString optionName = config->getConfig()->getAllOptionNames(nodeType, guestInterface).at(index.column());
    return config->getOption(nodeType, guestInterface, optionName, QVariant());
}

QVariant GuestInterfaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
        return config->getConfig()->getAllOptionNames(nodeType, "*").at(section);
    else
        return QVariant(section);
}


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
}

void NetworkPage::setupModels()
{
    guestModel = new GuestInterfaceModel(config, QString("net-guest"), this);
    guestView->setModel(guestModel);
    //guestView->setRootIndex(guestModel);
}

void NetworkPage::registerObjects()
{
    config->registerObject(networkCheck, "network", QVariant(true));
    config->registerObject(networkEdit, "networkCustomOptions");
}











