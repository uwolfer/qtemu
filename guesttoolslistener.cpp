#include "guesttoolslistener.h"
#include "GuestTools/modules/guestmodule.h"
#include "GuestTools/modules/clipboardsync.h"
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
	*stream >> type;
	*stream >> data;
    for(int i = 0; i < modules.size(); i++)
    {
    	if(modules.at(i)->moduleName() == usesModule)
    	{
    		modules.at(i)->receiveData(type, data);
    	}
    }
}

void GuestToolsListener::addModules()
{
    modules.append(new ClipboardSync(stream, this));
}

void GuestToolsListener::dataSender(QString module, QString type, QVariant data)
{
    sender()->blockSignals(true);
	*stream << module << type << data;
	sender()->blockSignals(false);

}

//
