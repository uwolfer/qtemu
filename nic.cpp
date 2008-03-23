//
// C++ Implementation: nic
//
// Description: provides a container for information about a particular NIC.
//
//
// Author: Ben Klopfenstein <benklop@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "nic.h"
#include "networksystem.h"
#include <QDateTime>
#include <QProcess>

Nic::Nic(const NicType & newType, const int & vLanReq, const QByteArray & mac, QObject * parent) : QObject(parent)
{
    new QProcess(this);
    if(mac != "random")
        setMacAddress(mac);
    setType(newType);
}

Nic::~Nic()
{
    setEnabled(false);
    vLanList.replace(vLan, vLanList.at(vLan) - 1);
}

bool Nic::isEnabled() const
{
    return enabled;
}


void Nic::setEnabled ( bool theValue )
{
    if(theValue == true)
        enabled = ifUp();
    else
        enabled = !ifDown();
}

QByteArray Nic::generateMacAddress()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    macAddress = "525400";//manufacturer part of the address -- from qemu
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
    switch(nicType)
    {
        case User:
            //user mode setup values- not much needed here
            break;
        case VLan:
            //vlan mode setup values- not implemented yet
            break;
        case Bridged:
            //bridged mode setup values
            setTapInterface();
            setBridgeInterface();
            break;
        case LocalBridged:
            setTapInterface();
            //local bridged mode setup values
            break;
        case Vde:
            //vde mode setup values - not implemented yet
            break;
        case Custom:
        default:
            //custom mode setup values - not implemented yet
            break;
    }
}


QByteArray Nic::getMacAddress()
{
    QByteArray formattedMac;
    if(macAddress == "")
        formattedMac = generateMacAddress();
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
        tapInterface = "qemu_" + macAddressPart;
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
        if(bridges.size() == 0)
            createBridge();
        bridgeInterface = bridges.first();
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

bool Nic::ifUp()
{
    bool success = true;
    optionList.clear();
    switch(nicType)
    {
        case User:
            //bring up user mode, set parameters
            optionList << "-net" << "nic" << "-net" << "user";
            break;
        case Bridged:
            setVLan();
            optionList << "-net" << "nic,macaddr=" + (QString)getMacAddress() + ",vlan=" + getVLan() << "-net" << "ifname=" + (QString)getTapInterface() + ",script=no";
            success = createTap();
            if(!bridgeExists())
                success = createBridge();
            success = connectTapToBridge();
            success = connectHardwareNicToBridge(hardwareInterface);
            break;
        case LocalBridged:
             setVLan();
             optionList << "-net" << "nic,macaddr=" + (QString)getMacAddress() + ",vlan=" + getVLan() << "-net" << "ifname=" + (QString)getTapInterface() + ",script=no";
            success = createTap();
            if(!bridgeExists())
                success = createBridge();
            success = connectTapToBridge();
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
    return success;
}

bool Nic::ifDown()
{
    return true;
}


QStringList Nic::getOptionList() const
{
    return optionList;
}

bool Nic::createTap()
{
    return true;
}

bool Nic::destroyTap()
{
    return true;
}

bool Nic::connectTapToBridge()
{
    return true;
}

bool Nic::removeTapFromBridge()
{
    return true;
}

bool Nic::bridgeExists()
{
    return true;
}

bool Nic::bridgeInUse()
{
    return false;
}

bool Nic::createBridge()
{
    return true;
}

bool Nic::destroyBridge()
{
    return true;
}

bool Nic::connectHardwareNicToBridge(const QByteArray & nicName)
{
    return true;
}

bool Nic::removeHardwareNicFromBridge(const QByteArray & nicName)
{
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
//static members

//static data
QList<int> Nic::vLanList;
QList<QByteArray> Nic::bridges;
