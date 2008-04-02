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

#include "nic.h"
#include "networksystem.h"
#include <QDateTime>
#include <QProcess>
#include <QRegExp>

Nic::Nic(const NicType & newType, const int & vLanReq, const QByteArray & mac, QObject * parent) : QObject(parent)
{
    findEnv();
    setMacAddress(mac);
    setType(newType);
}

Nic::~Nic()
{
    Enable(false);
    ifDown();
    if(initialized)
        deInitNic();
    vLanList.replace(vLan, vLanList.at(vLan) - 1);
}

void Nic::ifUp()
{
    if(!initialized && !initNic())
        return;

    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << ipPath << "link" << "set" << tapInterface << "up";
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
}

void Nic::ifDown()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << ipPath << "link" << "set" << tapInterface << "down";
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
}

QByteArray Nic::generateMacAddress()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    macAddress = "525400";//manufacturer part of the address -- from qemu's default
    macAddressPart = "";
    for(int i=0;i<6;i++)
    {
        macAddressPart = macAddressPart + QString::number(qrand()%16, 16).toUpper().toAscii();
    }
    macAddress = macAddress + macAddressPart;
    return macAddress;
}


NicType Nic::type() const
{
    return nicType;
}


void Nic::setType ( const NicType& theValue )
{
    nicType = theValue;
    initNic();

}

//doesnt work!
QByteArray Nic::getMacAddress()
{
    QByteArray formattedMac;
    if(macAddress == "")
        formattedMac = "000000000000";
    else
        formattedMac = macAddress;
    for(int i = 10;i>0;i = i - 2)//going backwards through the string makes it easier to calculate positions
    {
        formattedMac.insert(i,':');
    }
    return formattedMac;
}


void Nic::setMacAddress ( const QByteArray& theValue )
{
    if( theValue == "random" )
        generateMacAddress();
    else
    {
        QByteArray temp = theValue;
        for(int i=2;i<12;i = i + 2)
            temp.remove(i,1);
        macAddress = temp;
        macAddressPart = temp.mid(7,6);
    }
    setTapInterface();
}


int Nic::getVLan() const
{
    return vLan;
}


void Nic::setVLan ( int theValue )
{   //vLanList is a static member, so we basically have a log of what vlans are being used.
    //if we can find an unused one (0), we use it, otherwise we create another.
    if(theValue == -1)
        vLan = vLanList.indexOf(0);
    else
        vLan = theValue;
    if(vLan != -1)
    {
        vLanList.replace(vLan, vLanList.at(vLan) + 1);
    }
    else
    {
        vLanList.append(1);
        vLan = vLanList.size() - 1;
    }
}


QByteArray Nic::getTapInterface() const
{
    return tapInterface;
}


void Nic::setTapInterface ( const QByteArray& theValue )
{
    if(theValue != "")
        tapInterface = theValue;
    else
    {
        tapInterface = "qtemu-tap" + macAddressPart;
    }
}


QByteArray Nic::getBridgeInterface() const
{
    return bridgeInterface;
}


void Nic::setBridgeInterface ( const QByteArray& theValue )
{
    if(theValue != "")
        bridgeInterface = theValue;
    else
    {
        bridgeInterface = "qtemu-br" + macAddressPart;
        createBridge();
        bridgeInterface = bridges.last();
    }
}


bool Nic::operator ==(Nic otherNic)
{
    return (otherNic.getMacAddress() == getMacAddress());
}


int Nic::getNumVLanMembers() const
{
    return vLanList.at(vLan);
}

bool Nic::initNic()
{
    optionList.clear();
    bool success = false;
    switch(nicType)
    {
        case User:
            //bring up user mode, set parameters
            optionList << "-net" << "nic" << "-net" << "user";
            success = true;
            break;
        case Bridged:
            //bring up bridged mode, set parameters
            setVLan();
            optionList << "-net" << "nic,macaddr=" + (QString)getMacAddress() + ",vlan=" + getVLan() << "-net" << "ifname=" + (QString)getTapInterface() + ",script=no";
            if(
                createTap() &&
                createBridge() &&
                connectTapToBridge() &&
                connectHardwareNicToBridge()
               )
                success = true;
            break;
        case LocalBridged:
             //bring up local bridged mode, set parameters
             setVLan();
             optionList << "-net" << "nic,macaddr=" + (QString)getMacAddress() + ",vlan=" + getVLan() << "-net" << "ifname=" + (QString)getTapInterface() + ",script=no";
            if(
                createTap() &&
                createBridge() &&
                connectTapToBridge()
              )
                success = true;
            break;
        case VLan:
            //not implemented
            setVLan();
            break;
        case Vde:
            //not implemented
            break;
        case Custom:
        default:
            //not implemented
            break;
    }
    initialized = success;
    return success;
}

bool Nic::deInitNic()
{
    switch(nicType)
    {
        case Bridged:
            //bring down bridged mode
            removeHardwareNicFromBridge();
            removeTapFromBridge();
            destroyBridge();
            destroyTap();
            break;
        case LocalBridged:
            removeTapFromBridge();
            destroyBridge();
            destroyTap();
            break;
        default:
            //not implemented
            break;
    }
    return true;
}


QStringList Nic::getOptionList() const
{
    return optionList;
}

bool Nic::createTap()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << tunctlPath << "-u" << userName << "-t" << tapInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    else
        return true;
}

bool Nic::destroyTap()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << tunctlPath << "-d" << tapInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    else
        return true;
}

bool Nic::connectTapToBridge()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "addif" <<  bridgeInterface << tapInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    return true;
}

bool Nic::removeTapFromBridge()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "delif" <<  bridgeInterface << tapInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    return true;
}

bool Nic::bridgeExists()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "show";
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->readAllStandardOutput().contains(bridgeInterface))
        return true;
    return false;
}

bool Nic::bridgeInUse()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "show";
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    QString output = tempProcess->readAllStandardOutput();
    QStringList singleLines = output.split("\n");
    for(int i=0;i<singleLines.size();i++)
    {
        QStringList partsOfEach = singleLines.at(i).split("\t");
        if( partsOfEach.last().contains("[\\S]"))
        {
            return true;
        }
    }
    return false;
}

bool Nic::createBridge()
{
    if(bridgeExists())
    {
        bridges.append(bridgeInterface);
        return true;
    }
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "addbr" <<  bridgeInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    bridges.append(bridgeInterface);
    return true;
    
}

bool Nic::destroyBridge()
{
    if(bridgeInUse())
        return false;
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << brctlPath << "delbr" <<  bridgeInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    if(tempProcess->exitCode() == -1)
        return false;
    bridges.removeAll(bridgeInterface);
    return true;
}

bool Nic::connectHardwareNicToBridge()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    clearHwNic();
    HwNic thisNic;
    for(int i=0;i<hardwareNics.size();i++)
    {
        if(hardwareNics.at(i).kernelName == hardwareInterface)
            thisNic = hardwareNics.at(i);
    }
    
    tempOpts << brctlPath << "addif" <<  bridgeInterface << hardwareInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    
    tempOpts.clear();
    tempOpts << ipPath << "addr" << "add" << thisNic.HwIP + thisNic.HwCidr << "dev" << bridgeInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    
    tempOpts.clear();
    tempOpts << ipPath << "link" << "set" << bridgeInterface << "up";
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();

    return true;
}

bool Nic::removeHardwareNicFromBridge()
{
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    HwNic thisNic;
    for(int i=0;i<hardwareNics.size();i++)
    {
        if(hardwareNics.at(i).kernelName == hardwareInterface)
            thisNic = hardwareNics.at(i);
    }
    
    tempOpts << ipPath << "addr" << "del" << thisNic.HwIP + thisNic.HwCidr << "dev" << bridgeInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    
    tempOpts.clear();
    tempOpts << brctlPath << "delif" << bridgeInterface << hardwareInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    
    restoreHwNic();
    
    return true;
}

QByteArray Nic::getHardwareInterface() const
{
    return hardwareInterface;
}


void Nic::setHardwareInterface ( const QByteArray& theValue )
{
    hardwareInterface = theValue;
}

bool Nic::getLocalVLan() const
{
    return localVLan;
}


void Nic::setLocalVLan ( bool theValue )
{
    localVLan = theValue;
}

void Nic::findEnv()
{
    QProcess *tempProcess = new QProcess(this);
    QByteArray tempOutput;
    tunctlPath = sudoPath = ipPath = brctlPath = "unavailable";
    
    //TODO: load locations from program config here
    #ifndef Q_OS_WIN32
    
    tempProcess->start("which", QStringList("sudo"));
    tempProcess->waitForFinished();
    sudoPath = tempProcess->readAllStandardOutput().trimmed();
    
    #ifdef DEVELOPER
    qDebug("sudo path: " + sudoPath);
    #endif
    
    tempProcess->start(sudoPath, QStringList("-l"));
    tempProcess->waitForFinished();
    QStringList sudoList = ((QString)(tempProcess->readAllStandardOutput())).split('\n');
    QStringList sudoAllowed;
    for(int i=0;i<sudoList.size();i++)
    {
        if(sudoList.at(i).contains("NOPASSWD"))
        {
            sudoAllowed = sudoList.at(i).split(':').at(1).split(", ");
        }
    }
    #ifdef DEVELOPER
    qDebug("allowed sudo programs: " + sudoAllowed.join(", ").toAscii());
    #endif

    tempProcess->start("which", QStringList("brctl"));
    tempProcess->waitForFinished();
    tempOutput = tempProcess->readAllStandardOutput().trimmed();
    if(sudoAllowed.contains(tempOutput) && brctlPath == "unavailable")
        brctlPath = tempOutput;
    
    #ifdef DEVELOPER
    qDebug("brctl path: " + brctlPath);
    #endif
    
    tempProcess->start("which", QStringList("ip"));
    tempProcess->waitForFinished();
    tempOutput = tempProcess->readAllStandardOutput().trimmed();
    if(sudoAllowed.contains(tempOutput) && ipPath == "unavailable")
        ipPath = tempOutput;
    
    #ifdef DEVELOPER
    qDebug("ip path: " + ipPath);
    #endif
    
    tempProcess->start("which", QStringList("tunctl"));
    tempProcess->waitForFinished();
    tempOutput = tempProcess->readAllStandardOutput().trimmed();
    if(sudoAllowed.contains(tempOutput) && tunctlPath == "unavailable")
        tunctlPath = tempOutput;
    
    #ifdef DEVELOPER
    qDebug("tunctl path: " + tunctlPath);
    #endif
    
    //also get the user name for tunctl's use
    userName = qgetenv("USER");

    #elif defined(Q_OS_WIN32)
    //find needed executables in windows...
    #endif
    //save locations to config file
}

bool Nic::isEnabled() const
{
    return enabled;
}


void Nic::Enable ( bool theValue )
{
    enabled = theValue;
}

void Nic::clearHwNic()
{
    for(int i=0;i<hardwareNics.size();i++)
    {
        if(hardwareNics.at(i).kernelName == hardwareInterface)
            return;
    }
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    HwNic thisNic;
    thisNic.kernelName = hardwareInterface;
    
    tempOpts << ipPath << "addr" << "show" << hardwareInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    QString ipInfo = tempProcess->readAllStandardOutput();
    QRegExp ipAddrRegex("([0-9]{1,3}\\.){3}[0-9]{1,3}");
    QRegExp ipCidrRegex("/[0-9]{1,2}");
    ipAddrRegex.indexIn(ipInfo);
    ipCidrRegex.indexIn(ipInfo);
    thisNic.HwIP = ipAddrRegex.capturedTexts().at(1).toAscii();
    thisNic.HwNetmask = ipAddrRegex.capturedTexts().at(2).toAscii();
    thisNic.HwCidr = ipCidrRegex.capturedTexts().at(1).toAscii();
    hardwareNics.append(thisNic);
    
    tempOpts.clear();
    tempOpts << ipPath << "addr" << "del" << thisNic.HwIP + thisNic.HwCidr << "dev" << hardwareInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
}

void Nic::restoreHwNic()
{
    HwNic thisNic;
    for(int i=0;i<hardwareNics.size();i++)
    {
        if(hardwareNics.at(i).kernelName == hardwareInterface)
            thisNic = hardwareNics.at(i);
    }
    QProcess *tempProcess = new QProcess(this);
    QStringList tempOpts;
    tempOpts << ipPath << "addr" << "add" << thisNic.HwIP + thisNic.HwCidr << "dev" << hardwareInterface;
    tempProcess->start(sudoPath, tempOpts);
    tempProcess->waitForFinished();
    for(int i=0;i<hardwareNics.size();i++)
    {
        if(hardwareNics.at(i).kernelName == hardwareInterface)
            hardwareNics.removeAt(i);
    }	
}

//static data
QList<int> Nic::vLanList;
QList<QByteArray> Nic::bridges;
QList<HwNic> Nic::hardwareNics;
