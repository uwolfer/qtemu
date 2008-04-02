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

#ifndef NIC_H
#define NIC_H

#include <QObject>
#include <QStringList>

enum NicType {User, VLan, LocalBridged, Bridged, Vde, Custom};
struct HwNic 
{
    QByteArray kernelName;
    QByteArray HwIP;
    QByteArray HwNetmask;
    QByteArray HwCidr;
};


/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/

class Nic : public QObject
{
        Q_OBJECT
    public:

        bool operator == ( Nic otherNic );

        Nic ( const NicType& newType, const int & vLanReq = -1, const QByteArray & mac = "random", QObject *parent = 0 );

        Nic ( QObject *parent = 0 );

        ~Nic();
        void ifUp();
        void ifDown();
        void setType ( const NicType& theValue );
        NicType type() const;
        void setMacAddress ( const QByteArray& theValue = "random" );
        QByteArray getMacAddress();
        void setVLan ( int theValue = -1 );
        int getVLan() const;
        void setTapInterface ( const QByteArray& theValue = "");
        QByteArray getTapInterface() const;
        void setBridgeInterface ( const QByteArray& theValue = "" );
        QByteArray getBridgeInterface() const;
        int getNumVLanMembers() const;
	QStringList getOptionList() const;
	void setHardwareInterface ( const QByteArray& theValue );
	QByteArray getHardwareInterface() const;
	void setLocalVLan ( bool theValue );
	bool getLocalVLan() const;
	bool initNic();
        bool deInitNic();

	void Enable ( bool theValue );
	

	bool isEnabled() const;
	
    private:
//methods
        QByteArray generateMacAddress();
        bool createTap();
        bool destroyTap();
        bool connectTapToBridge();
        bool removeTapFromBridge();
        bool bridgeExists();
        bool bridgeInUse();
        bool createBridge();
        bool destroyBridge();
        bool connectHardwareNicToBridge();
        bool removeHardwareNicFromBridge();
        void findEnv();
        void clearHwNic();
        void restoreHwNic();
//data
        NicType nicType;
        QByteArray macAddress;
        QByteArray macAddressPart;
        int vLan;
        QByteArray tapInterface;
        QByteArray bridgeInterface;
        QByteArray hardwareInterface;
        bool enabled;
        bool initialized;
        bool localVLan;
        QStringList optionList;
        QByteArray userName;
        QByteArray brctlPath;
        QByteArray ipPath;
        QByteArray tunctlPath;
        QByteArray sudoPath;
//static data
        static QList<int> vLanList;
        static QList<QByteArray> bridges;
        static QList<HwNic> hardwareNics;
};



#endif
