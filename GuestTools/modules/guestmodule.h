/*
 * guestmodule.h
 *
 *  Created on: Dec 14, 2008
 *      Author: Ben
 */

#ifndef GUESTMODULE_H_
#define GUESTMODULE_H_

#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QString>

class GuestModule: public QObject
{
    Q_OBJECT

public:
	GuestModule(QDataStream *stream, QObject *parent = 0);
	virtual ~GuestModule();

	virtual void receiveData(QString type, QVariant data);

	QString moduleName();

protected:
    QDataStream *stream;

    void send(QString type, QVariant data);
    void setModuleName(QString name);

private:
	QString module;

signals:
    void sendData(QString module, QString type, QVariant data);

};

#endif /* GUESTMODULE_H_ */
