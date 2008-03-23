//
// C++ Interface: networksystem
//
// Description: 
//
//
// Author: Ben Klopfenstein <benklop@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NETWORKSYSTEM_H
#define NETWORKSYSTEM_H

#include "nic.h"
#include <QObject>
#include <QList>

/**
    @author Ben Klopfenstein <benklop@gmail.com>
*/

class NetworkSystem : public QObject
{
Q_OBJECT
public:
    NetworkSystem(QObject *parent = 0);

    ~NetworkSystem();
    int numNics();
    
    QList<Nic *> getNicByType(NicType nicType);
    Nic * getNicByMac(QByteArray mac);

public slots:
    void clearAllNics();
    QByteArray makeNewNic(NicType type, int vLan = -1);//returns nic mac
    void initializeNic(QByteArray macAddress);
    

private:
    QList<Nic *> nicList;

};
#endif
