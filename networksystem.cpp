/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop @ gmail.com>
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

#include "networksystem.h"

NetworkSystem::NetworkSystem(QObject *parent)
 : QObject(parent)
{
}


NetworkSystem::~NetworkSystem()
{
}

int NetworkSystem::numNics()
{
    return nicList.size();
}

QByteArray NetworkSystem::makeNewNic(NicType type, int vLan)
{
    nicList.append(new Nic(type, vLan , "random", this));
    return nicList.last()->getMacAddress();
}


void NetworkSystem::clearAllNics()
{
    nicList.clear();
}

QList<Nic *> NetworkSystem::getNicByType(NicType nicType)
{
    QList<Nic *> smNicList;
    for(int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i)->type() == nicType)
            smNicList.append(nicList.at(i));
    }
    return smNicList;
}

Nic * NetworkSystem::getNicByMac(const QByteArray mac)
{
    Nic * smNic = 0;
    for(int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i) == mac)
        smNic = nicList.at(i);
    }
    return smNic;
}


void NetworkSystem::initializeNic(const QByteArray macAddress)
{
    Nic* currentNic = getNicByMac(macAddress);
    currentNic->initNic();
}

void NetworkSystem::saveNics()
{
    //get info from all nics and save to an XML file
}

void NetworkSystem::loadNics()
{
    //get info from an XML file and use it to create all nics
    //if there is no network section, create the default nic. 
    
    //for the moment just create the default nic
    makeNewNic(User,0);
}

QStringList NetworkSystem::getOptionList()
{
    QStringList optionList;
    if(nicList.size() == 0)
        return QStringList("");
    for (int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i)->isEnabled())
            optionList << nicList.at(i)->getOptionList();
    }
    return optionList;
}

void NetworkSystem::delNic( const QByteArray mac)
{
    Nic * smNic = 0;
    for(int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i)->getMacAddress() == mac)
            nicList.removeAt(i);
    }
}

void NetworkSystem::findHardwareNics()
{
    //fill the hardwareNics qlist
}

void NetworkSystem::addSambaDir( const QString dir)
{
}
