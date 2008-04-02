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
    Nic * smNic = 0;
    for(int i=0;i<nicList.size();i++)
    {
        if(nicList.at(i) == mac)
        smNic = nicList.at(i);
    }
    return smNic;
}


void NetworkSystem::initializeNic(QByteArray macAddress)
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

void NetworkSystem::delNic(QByteArray mac)
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

void NetworkSystem::addSambaDir(QString dir)
{
}
