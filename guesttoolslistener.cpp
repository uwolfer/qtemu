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

#include "guesttoolslistener.h"
#include "GuestTools/modules/clipboard/clipboardsync.h"
#include <QLocalSocket>
#include <QDataStream>
#include <QVariant>
//
GuestToolsListener::GuestToolsListener( QString location, QObject *parent ) 
	: QObject(parent)
{
    blockSize = 0;
    toolSocket = new QLocalSocket(this);
    //connect(toolSocket, SIGNAL(connected()), this, SLOT(setupConnection()));
    //server = new QLocalServer(this);
    toolSocket->connectToServer(location, QIODevice::ReadWrite);
    qDebug() << "connecting to" << location;
    setupConnection();
}

void GuestToolsListener::setupConnection()
{
    qDebug() << "setting up guest tools";
    addModules();
    connect(toolSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

void GuestToolsListener::receiveData()
{
    //connect the stream
    QDataStream stream(toolSocket);
    stream.setVersion(QDataStream::Qt_4_0);
    //get the size of the data chunk
    if (blockSize == 0) {
        if (toolSocket->bytesAvailable() < (int)sizeof(quint64))
            return;
        stream >> blockSize;
    }

    //don't continue until we have all the data
    if ((quint64)(toolSocket->bytesAvailable()) < blockSize)
        return;

    QString usesModule;
    QVariant data;


    stream >> usesModule >> data;

    blockSize = 0;

    for(int i = 0; i < modules.size(); i++)
    {
    	if(modules.at(i)->moduleName() == usesModule)
    	{
            qDebug() << "received data from"<< usesModule;

            modules.at(i)->receiveData(data);
            return;
    	}
    }
    qDebug() << "invalid module" << usesModule;
}

void GuestToolsListener::addModules()
{
    modules.append(new ClipboardSync(this));
}

void GuestToolsListener::dataSender(QString module, QVariant &data)
{
    //so that we don't try to send more than one at a time
    //sender()->blockSignals(true);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint64)0;
    out << module;
    out << data;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));

    toolSocket->write(block);

    //re-allow signals
    //sender()->blockSignals(false);

}

//
