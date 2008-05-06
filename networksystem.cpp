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

void NetworkSystem::makeNewNic(const NicType type, const int vLan, const QByteArray mac)
{
    nicList.append(new Nic(type, vLan , mac, this));
}


void NetworkSystem::clearAllNics()
{
    if(nicList.isEmpty())
        return;
    qDeleteAll(nicList);
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


void NetworkSystem::loadNics()
{
    //get info from an XML file and use it to create all nics
    //if there is no network section, create the default nic. 
    
    //TODO: load data from cfg
    //find the number of nics in the cfg
    int numOfCfgNics = 1;
    
    //for each nic in the cfg
    for(int i=0;i<numOfCfgNics;i++)
    {
        /******load data******/
        //TODO: load data from cfg rather than static data...
        //if the nic is not enabled break
        bool enabledFromCfg = true;
        if( !enabledFromCfg)
            break;
        //load the nic's type
        
        
        //NicType typeFromCfg = Bridged;//Bridged, User, LocalBridged
        NicType typeFromCfg = User;//Bridged, User, LocalBridged
        
        
        //load the nic's mac
        QByteArray macFromCfg = "random";
        //load the nic's vlan
        int vlanFromCfg = -1;
        
        //load the nic's Bridging settings if it is a bridge
        //declared here to make sure thay are in the right scope
        QByteArray hwNicFromCfg;
        QByteArray tapNameFromCfg;
        QByteArray bridgeNameFromCfg;
        QByteArray bridgeAddressFromCfg;
        QString customUpScriptFromCfg;
        QString customDownScriptFromCfg;
        switch(typeFromCfg)
        {
            case Bridged:
                //load the nic bridge's hardware interface
                hwNicFromCfg = "eth0";
            case LocalBridged:
                //load bridge settings from cfg
                tapNameFromCfg = "auto";
                bridgeNameFromCfg = "auto";
                break;
            case Custom:
                //load the script custom runs from cfg
                customUpScriptFromCfg = "custom_ifup.sh";
                customDownScriptFromCfg = "custom_ifdown.sh";
                break;
            default:
                break;
        }
        
        /******initialization******/

        //initialize the new nic
        makeNewNic(typeFromCfg, vlanFromCfg, macFromCfg);
        //do things depending on the type
        switch(typeFromCfg)
        {
            case Bridged:
                nicList.last()->setHardwareInterface(hwNicFromCfg);
            case LocalBridged:
                if(tapNameFromCfg!="auto")
                    nicList.last()->setTapInterface(tapNameFromCfg);
                if(bridgeNameFromCfg!="auto")
                    nicList.last()->setBridgeInterface(bridgeNameFromCfg);
                else
                    nicList.last()->setBridgeInterface();
                break;
            case Custom:
                //set ifup script
                //set ifdown script
                break;
            default:
                break;
        }
        //all configured nics loaded
        
        //init the current nic
        nicList.last()->ifUp();
        
    }
    //samba sharing requires a user nic, so we need to check for one here
    if(!sambaDir.isEmpty() && getNicByType(User).isEmpty())
        makeNewNic(User, -1);
}

QStringList NetworkSystem::getOptionList()
{
    QStringList optionList;
    if(nicList.size() == 0)
        return QStringList("");
    for (int i=0;i<nicList.size();i++)
        optionList << nicList.at(i)->getOptionList();
    
    if(!sambaDir.isEmpty())
        optionList << "-smb" << sambaDir;
    return optionList;
}

void NetworkSystem::addSambaDir( const QString dir)
{
    sambaDir = dir;
}

