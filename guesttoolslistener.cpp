#include "guesttoolslistener.h"
#include "GuestTools/modules/clipboard/clipboardsync.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QVariant>
//
GuestToolsListener::GuestToolsListener( QString location, QObject *parent ) 
	: QObject(parent)
{
    server = new QLocalServer(this);
    server->listen(location);
    qDebug() << "listening on" << location;
    connect(server, SIGNAL(newConnection()), this, SLOT(setupConnection()));
}

void GuestToolsListener::setupConnection()
{
    QLocalSocket *device = server->nextPendingConnection();
    stream = new QDataStream(device);
    
    addModules();
    connect(stream->device(), SIGNAL(readyRead()), this, SLOT(receiveData()));
}

void GuestToolsListener::receiveData()
{
	QString usesModule;
	QString type;
	QVariant data;
	*stream >> usesModule;
        
    qDebug() << "received data from"<< usesModule;
    
    for(int i = 0; i < modules.size(); i++)
    {
    	if(modules.at(i)->moduleName() == usesModule)
    	{
   	        *stream >> type;
	        *stream >> data;
    		modules.at(i)->receiveData(type, data);
    	}
    }
}

void GuestToolsListener::addModules()
{
    modules.append(new ClipboardSync(stream, this));
}

void GuestToolsListener::dataSender(QString module, QString type, QVariant &data)
{
    //if(data.isNull()||module.isNull()||type.isNull())
        //return;
    sender()->blockSignals(true);
    qDebug() << module;
    qDebug() << type;
    qDebug() << data;
	*stream << module << type << data;
	sender()->blockSignals(false);

}

//
