//
// C++ Implementation: networksystem
//
// Description: Holds multiple NICs in a VM and does administration to bring them up and down
//
//
// Author: Ben Klopfenstein <benklop@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

Nic * NetworkSystem::getNicByMac(QByteArray mac)
{
    Nic * smNic;
    for(int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i)->getMacAddress() == mac)
            smNic = nicList.at(i);
    }
    return smNic;
}


void NetworkSystem::initializeNic(QByteArray macAddress)
{
    Nic* currentNic = getNicByMac(macAddress);
}
