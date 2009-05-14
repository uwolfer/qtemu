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
** C++ Interface: interfacemodel
**
** Description: provides a model for the model/view framework to describe the
** various options for a qemu network interface, both guest and host models
** are provided.
**
****************************************************************************/

#ifndef INTERFACEMODEL_H
#define INTERFACEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class MachineConfigObject;


/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class InterfaceModel : public QAbstractTableModel
{
Q_OBJECT
public:
    InterfaceModel(MachineConfigObject *config, QString nodeType,  QObject *parent = 0);

    int rowCount(const QModelIndex & parent = QModelIndex() ) const;
    int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex &index ) const;
    bool setData( const QModelIndex & index, const QVariant& value, int role );
     
    QString rowName(int row) const;
    QString colName(int col) const;
protected:
    MachineConfigObject *config;
    QString nodeType;
    //first string in the list is the key
    QStringList columns;

protected slots:
    void optionChanged(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);
};

class GuestInterfaceModel : public InterfaceModel
{
Q_OBJECT
public:
    explicit GuestInterfaceModel(MachineConfigObject *config, QObject *parent = 0);

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());

    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
};

class HostInterfaceModel : public InterfaceModel
{
Q_OBJECT
public:
    explicit HostInterfaceModel(MachineConfigObject *config, QObject *parent = 0);

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());

    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
};
#endif
