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
#include "machineconfigobject.h"
#include "netconfig.h"
#include <QByteArray>
#include <QDebug>
#include <stdlib.h>
#include <QTime>

//Main NetConfig Class
NetConfig::NetConfig(QObject *parent, MachineConfigObject *config) 
 : QObject(parent)
 , config(config)
{
    hostIfs = new QList<HostInterface*>;
    guestIfs = new QList<GuestInterface*>;
    hostIfNames = QStringList();
}

void NetConfig::buildIfs()
{
    hostIfs->clear();
    guestIfs->clear();
    
    QStringList guestNames = config->getConfig()->getAllOptionNames("net-guest", "");
    QStringList hostNames = config->getConfig()->getAllOptionNames("net-host", "");
    
    for(int i=0;i<hostNames.size();i++)
    {
        hostIfs->append(new HostInterface(this,hostNames.at(i)));
        hostIfs->last()->setVlan(i);
        hostIfNames << hostIfs->last()->property("name").toString();
    }
    for(int i=0;i<guestNames.size();i++)
    {
        guestIfs->append(new GuestInterface(this,guestNames.at(i)));
        if(guestIfs->last()->property("host").isValid())
        {
            guestIfs->last()->setVlan(hostIfs->at(hostIfNames.indexOf(guestIfs->last()->property("host").toString()))->getVlan());
            
        }
        else
        {
            guestIfs->last()->setVlan(-1);
            guestIfs->last()->setProperty("enabled", false);
        }
            
    }
}

QStringList NetConfig::getOptionString()
{
    buildIfs();
    QStringList opts;
    QList<NetInterface*> usedHostIfs;
    for(int i=0;i<guestIfs->size();i++)
    {
        if(guestIfs->at(i)->property("enabled").toBool())
        {
            opts << guestIfs->at(i)->parseOpts();
            usedHostIfs.append(hostIfs->at(hostIfNames.indexOf(guestIfs->at(i)->property("host").toString())));
        }
    }
    for(int i=0;i<usedHostIfs.size();i++)
    {
        opts << usedHostIfs.at(i)->parseOpts();
    }
    return opts;
}


//Basic NetInterface Class
NetInterface::NetInterface(NetConfig *parent, QString nodeType, QString nodeName) 
 : QObject(parent)
 , config(parent->config)
 , nodeType(nodeType)
 , nodeName(nodeName)
{
    config->registerObject(this, nodeType, nodeName);
}

int NetInterface::getVlan()
{
    return vlan;
}

void NetInterface::setVlan(int number)
{
    vlan = number;
}

QStringList NetInterface::parseOpts()
{
    return QStringList();
}

GuestInterface::GuestInterface(NetConfig *parent, QString nodeName) 
 : NetInterface(parent, QString("net-guest"), nodeName)
{
    
}

QStringList GuestInterface::parseOpts()
{
    static bool firstTime = true;
    //need to come up with the mac properly.
    QString mac;
    if (property("randomize").toBool() || property("mac").toString() == tr("random"))
    {
        if (firstTime) 
        {
            firstTime = false;
            QTime midnight(0, 0, 0);
            qsrand(midnight.secsTo(QTime::currentTime()));
        }
        mac="52:54:00:";
        for (int i=1;i<=6;i++)
        {
            mac.append(QString().setNum(qrand() % 16, 16));
            if(i%2 == 0 && i != 6)
                mac.append(":");
        }
        setProperty("mac", mac);
        
    }
    else
    {
        mac = property("mac").toString();
    }
    
    QStringList opts;
    opts << "-net" << "nic,vlan=" + QString().setNum(vlan) + ",macaddr=" + mac + ",model=" + property("nic").toString();
    return opts;
}
     


HostInterface::HostInterface(NetConfig *parent, QString nodeName) 
 : NetInterface(parent, QString("net-host"), nodeName)
{
    
}

QStringList HostInterface::parseOpts()
{
    QString type;
    QStringList netOpts;
    QStringList opts;
    //FIXME: this will not work for a translated gui??
    if(property("type").toString() == tr("User Mode"))
    {
        //-net user[,vlan=n][,hostname=name]
        type="user";
        //additional options for this type:
        netOpts << "hostname=" + property("hostname").toString();
        //tftp and bootp
        if(property("bootp").toBool())
            opts << "-bootp" << property("bootpPath").toString();
        if(property("tftp").toBool())
            opts << "-tftp" << property("tftpPath").toString();
        //TODO: add SMB support
    }
    else if(property("type").toString() == tr("Bridged Interface"))
    {
        //-net tap[,vlan=n][,fd=h][,ifname=name][,script=file]
        type="tap";
        //additional options for this type:
        //[fd=h]
        netOpts << "ifname=" + property("interface").toString();
        //[script=file]
        netOpts << "script=no";
        netOpts << "downscript=no";
    }
    else if(property("type").toString() == tr("Routed Interface"))
    {
        //-net tap[,vlan=n][,fd=h][,ifname=name][,script=file]
        type="tap";
        //additional options for this type:
        //[fd=h]
        netOpts << "ifname=" + property("interface").toString();
        //[script=file]
        netOpts << "script=no";
        netOpts << "downscript=no";
    }
    else if(property("type").toString() == tr("Shared Virtual Lan"))
    {
        type="socket";
        if(property("vlanType").toString() == "udp")
        {
            //-net socket[,vlan=n][,fd=h][,mcast=maddr:port]
            netOpts << "mcast=" + property("address").toString() + ':' + property("port").toString();
        }
        else
        {
            //-net socket[,vlan=n][,fd=h][,listen=[host]:port][,connect=host:port]
            qDebug() << "tcp not yet supported";
        }
    }
    else if(property("type").toString() == tr("Custom TAP"))
    {
        //-net tap[,vlan=n][,fd=h][,ifname=name][,script=file]
        type="tap";
        netOpts << "ifname=" + property("interface").toString();
        netOpts << "script=" + property("ifUp").toString();
        netOpts << "downscript=" + property("ifDown").toString();
    }
    
    opts << "-net" << type + ",vlan=" + QString().setNum(vlan) + (netOpts.isEmpty()?QString():(',' + netOpts.join(",")));
    return opts;
}

HostActionItem::HostActionItem(NetConfig *parent, HostAction action, HostItem item, QString interface, QString interfaceTo)
 :QObject(parent)
 ,action(action)
 ,item(item)
 ,interface(interface)
 ,toInterface(interfaceTo)
{
    
}

