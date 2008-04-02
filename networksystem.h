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

#ifndef NETWORKSYSTEM_H
#define NETWORKSYSTEM_H
#include "config.h"
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
        NetworkSystem ( QObject *parent = 0 );

        ~NetworkSystem();
        int numNics();

        QList<Nic *> getNicByType ( NicType nicType );
        Nic * getNicByMac ( QByteArray mac );
        QStringList getOptionList();

    public slots:
        void clearAllNics();
        QByteArray makeNewNic ( NicType type, int vLan = -1 );//returns nic mac
        void delNic ( QByteArray macAddress );
        void initializeNic ( QByteArray macAddress );
        void saveNics();
        void loadNics();
        void addSambaDir(QString dir);

    private:
        QList<Nic *> nicList;
        QList<QByteArray> hardwareNics;

        void findHardwareNics();

};
#endif
