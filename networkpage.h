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
#include <QAbstractTableModel>
#include <QModelIndex>

#include "ui_networkpage.h"

class MachineConfigObject;

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/

/****************************************************************************
** C++ Interface: GuestInterfaceModel
**
** Description: a model for guest interfaces
**
****************************************************************************/
class GuestInterfaceModel : public QAbstractTableModel
{
Q_OBJECT
public:
    GuestInterfaceModel(MachineConfigObject *config, QString nodeType,  QObject *parent = 0);

     int rowCount(const QModelIndex & parent = QModelIndex() ) const;
     int columnCount(const QModelIndex & parent = QModelIndex() ) const;
     QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
     QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
private:
    MachineConfigObject *config;
    QString nodeType;
};

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
    NetworkPage(MachineConfigObject *config, QWidget *parent = 0);

    ~NetworkPage();

private:
    void makeConnections();
    void setupPage();
    void setupModels();
    void registerObjects();
    MachineConfigObject *config;

    GuestInterfaceModel *guestModel;

//dealing with model/view

private slots:
    void changeNetPage(bool state);

};


//class hostInterfaceModel;

#endif
