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
    connect(config->getConfig(), SIGNAL(optionChanged(const QString&, const QString&, const QString&, const QVariant&)), this, SLOT(optionChanged(const QString&, const QString&, const QString&, const QVariant&)));
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
    return columns.size();
}

QVariant InterfaceModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid())
        return QVariant();
    //maybe this could be optimized by caching the stringlists...
    QString nodeName = rowName(index.row());
    QString optionName = colName(index.column());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return config->getOption(nodeType, nodeName, optionName, QVariant());
    else
        return QVariant();
}

QVariant InterfaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
            return columns.at(section);
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
        QString nodeName = rowName(index.row());
        QString optionName = colName(index.column());
        config->getConfig()->setOption(nodeType, nodeName, optionName, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;

}

QString InterfaceModel::rowName(int row) const
{
    QStringList names = config->getConfig()->getAllOptionNames(nodeType, "");
    if(row <= names.size())
        return names.at(row);
    else
        return QString();
}

QString InterfaceModel::colName(int col) const
{
    return(columns.at(col));
}

void InterfaceModel::optionChanged(const QString & nodeType, const QString & nodeName, const QString & optionName, const QVariant & value)
{
    if(nodeType == this->nodeType)
    {
        reset();
    }
}

GuestInterfaceModel::GuestInterfaceModel(MachineConfigObject * config, QObject * parent)
  : InterfaceModel(config, QString("net-guest"), parent)
{
    columns << "name" << "mac";
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
        //set all options
        config->setOption(nodeType, nodeName, "name", QString(QString("Interface ") + QString::number(interfaceNumber)));
        config->setOption(nodeType, nodeName, "nic", "rtl8139");
        config->setOption(nodeType, nodeName, "mac", "random");
        config->setOption(nodeType, nodeName, "randomize", false);
        config->setOption(nodeType, nodeName, "host", QString());
    }
    endInsertRows();
    return true;

}

bool GuestInterfaceModel::removeRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    QString nodeName;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < (row + count); i++)
    {
        nodeName = config->getConfig()->getAllOptionNames(nodeType, "").at(i);
        config->getConfig()->clearOption(nodeType, "", nodeName);
    }

    endRemoveRows();
    return true;
}

HostInterfaceModel::HostInterfaceModel(MachineConfigObject * config, QObject * parent)
  : InterfaceModel(config, QString("net-host"), parent)
{
    columns << "name" << "type";
}

bool HostInterfaceModel::insertRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    QString nodeName;
    int interfaceNumber = 0;
    beginInsertRows(parent, row, count);
    for (int i=0; i<count; i++)
    {
        for(;config->getOption(nodeType, "", QString("host" + QString::number(interfaceNumber)), QVariant()).isValid();interfaceNumber++);
        nodeName = "host" + QString::number(interfaceNumber);
        //set all options
        config->setOption(nodeType, nodeName, "name", QString(QString("Interface ") + QString::number(interfaceNumber)));
        config->setOption(nodeType, nodeName, "type", "User Mode");
        //FIXME: use a better unique interface/bridge name
        config->setOption(nodeType, nodeName, "interface", "qtemu-tap" + QString::number(interfaceNumber));
        config->setOption(nodeType, nodeName, "bridgeInterface", "qtemu-br" + QString::number(interfaceNumber));
        config->setOption(nodeType, nodeName, "hardwareInterface", "eth0");
        config->setOption(nodeType, nodeName, "spanningTree", false);
        config->setOption(nodeType, nodeName, "ifUp", QString());
        config->setOption(nodeType, nodeName, "ifDown", QString());
        config->setOption(nodeType, nodeName, "hostname", "qtemu_guest");
        config->setOption(nodeType, nodeName, "tftp", false);
        config->setOption(nodeType, nodeName, "tftpPath", QString());
        config->setOption(nodeType, nodeName, "bootp", false);
        config->setOption(nodeType, nodeName, "bootpPath", QString());
        config->setOption(nodeType, nodeName, "vlanType", "udp");
        config->setOption(nodeType, nodeName, "address", "127.0.0.1");
        config->setOption(nodeType, nodeName, "port", "9000");
    }
    endInsertRows();
    return true;

}

bool HostInterfaceModel::removeRows(int row, int count, const QModelIndex & parent)
{
    Q_UNUSED(parent);
    QString nodeName;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < (row + count); i++)
    {
        nodeName = config->getConfig()->getAllOptionNames(nodeType, "").at(i);
        config->getConfig()->clearOption(nodeType, "", nodeName);
    }

    endRemoveRows();
    return true;

}





