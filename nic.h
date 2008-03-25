//
// C++ Interface: nic
//
// Description: provides a container for information about a particular NIC.
//
//
// Author: Ben Klopfenstein <benklop@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
