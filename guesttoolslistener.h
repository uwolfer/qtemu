#ifndef GUESTTOOLSLISTENER_H
#define GUESTTOOLSLISTENER_H
//
#include <QObject>
#include <QList>
class QLocalServer;
class QDataStream;
class GuestModule;
//
class GuestToolsListener : public QObject
{
Q_OBJECT
public:
	GuestToolsListener(QString location, QObject *parent);

private:
    QLocalServer *server;
    QDataStream *stream;
    QList<GuestModule *> modules;
    
    void addModules();
    
public slots:
    void dataSender(QString module, QString type, QVariant data);
    
private slots:
    void setupConnection();	
    void receiveData();
};
#endif
