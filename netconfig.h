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

#ifndef NETCONFIG_H
#define NETCONFIG_H
//
#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>

class MachineConfigObject;
class GuestInterface;
class HostInterface;
class HostActionItem;

class NetConfig : public QObject
{
Q_OBJECT
public:
    explicit NetConfig(QObject *parent = 0, MachineConfigObject *config = 0);
    
    MachineConfigObject *config;
    
    QStringList getOptionString();

    
private:
    void buildIfs();
    QList<GuestInterface*> *guestIfs;
    QList<HostInterface*> *hostIfs;
    QStringList hostIfNames;
    
    QList<HostActionItem*> *startActions;
    QList<HostActionItem*> *stopActions;
};

class NetInterface : public QObject
{
Q_OBJECT
public:
    explicit NetInterface(NetConfig *parent = 0, QString nodeType = QString(), QString nodeName = QString());
    
    int getVlan();
    void setVlan(int number);
    virtual QStringList parseOpts();
    int vlan;
private:

protected:
    MachineConfigObject *config;

    QString nodeType;
    QString nodeName;
    QString type;
};

class GuestInterface : public NetInterface
{
Q_OBJECT
public:
    explicit GuestInterface(NetConfig *parent = 0, QString nodeName = QString());
    virtual QStringList parseOpts();
};

class HostInterface : public NetInterface
{
Q_OBJECT
public:
    explicit HostInterface(NetConfig *parent = 0, QString nodeName = QString());
    virtual QStringList parseOpts();
};



class HostActionItem : public QObject
{
    enum HostAction { Add, RemoveIfUnused, Remove };
    enum HostItem { Bridge, Tap, Connection, Route};
Q_OBJECT
public:
    HostActionItem(NetConfig *parent, HostAction action, HostItem item, QString interface, QString interfaceTo = QString());
    
    bool performAction();
private:
    HostAction action;
    HostItem item;
    QString interface;
    QString toInterface;
};
#endif
