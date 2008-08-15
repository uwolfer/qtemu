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
** C++ Implementation: interfacemodel
**
** Description: 
**
****************************************************************************/
#include <QStringList>

#include "interfacemodel.h"
#include "machineconfigobject.h"

InterfaceModel::InterfaceModel(MachineConfigObject * config, QString nodeType, QObject * parent)
  : QAbstractTableModel(parent)
  , config(config)
  , nodeType(nodeType)
{
}

int InterfaceModel::rowCount(const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;
    int rows = config->getConfig()->getNumOptions(nodeType, "");
    //qDebug("rows %i", rows);
    return rows;
}

int InterfaceModel::columnCount(const QModelIndex & parent) const
{
    if(parent.isValid())
        return 0;
    int cols = config->getConfig()->getNumOptions(nodeType, "*");
    return cols;
}

QVariant InterfaceModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid())
        return QVariant();
    //maybe this could be optimized by caching the stringlists...
    QString guestInterface = config->getConfig()->getAllOptionNames(nodeType, "").at(index.row());
    QString optionName = config->getConfig()->getAllOptionNames(nodeType, guestInterface).at(index.column());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return config->getOption(nodeType, guestInterface, optionName, QVariant());
    else
        return QVariant();
}

QVariant InterfaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
            return config->getConfig()->getAllOptionNames(nodeType, "*").at(section);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


Qt::ItemFlags InterfaceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool InterfaceModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole)) 
    {
        QString nodeName = config->getConfig()->getAllOptionNames(nodeType, "").at(index.row());
        QString optionName = config->getConfig()->getAllOptionNames(nodeType, nodeName).at(index.column());
        config->getConfig()->setOption(nodeType, nodeName, optionName, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;

}


GuestInterfaceModel::GuestInterfaceModel(MachineConfigObject * config, QObject * parent)
  : InterfaceModel(config, QString("net-guest"), parent)
{
}


bool GuestInterfaceModel::insertRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    QString nodeName;
    int interfaceNumber = 0;
    beginInsertRows(parent, row, row + count - 1);
    for (int i=0; i<count; i++)
    {
        for(;config->getOption(nodeType, "", QString("guest" + QString::number(interfaceNumber)), QVariant()).isValid();interfaceNumber++);
        nodeName = "guest" + QString::number(interfaceNumber);
        qDebug("using guest interface %i", interfaceNumber);
        //set all options
        config->setOption(nodeType, nodeName, "name", QString(QString("Interface ") + QString::number(interfaceNumber)));
        config->setOption(nodeType, nodeName, "nic", "rtl8139");
        config->setOption(nodeType, nodeName, "mac", "random");
        config->setOption(nodeType, nodeName, "randomize", false);
    }
    endInsertRows();
    return true;

}

bool GuestInterfaceModel::removeRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    
    beginRemoveRows(parent, row, row + count - 1);
    for (int i=0; i<count; i++)
    {
        QString nodeName = config->getConfig()->getAllOptionNames(nodeType, "").at(row + count - 1);
        config->getConfig()->clearOption(nodeType, "", nodeName);
    }
    endRemoveRows();
    return true;
}


