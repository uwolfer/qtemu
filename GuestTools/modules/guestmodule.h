/*
 * guestmodule.h
 *
 *  Created on: Dec 14, 2008
 *      Author: Ben Klopfenstein
 */

#ifndef GUESTMODULE_H_
#define GUESTMODULE_H_

#include <QObject>
#include <QVariant>
#include <QString>

class GuestModule: public QObject
{
    Q_OBJECT

public:
        GuestModule(QObject *parent = 0);
	virtual ~GuestModule();

        virtual void receiveData(QVariant data);

	QString moduleName();

protected:

    void send(QVariant &data);
    void setModuleName(QString name);

private:
	QString module;

signals:
    void sendData(QString module, QVariant &data);

};

#endif /* GUESTMODULE_H_ */
