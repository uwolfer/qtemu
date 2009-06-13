/*
 * guestmodule.cpp
 *
 *  Created on: Dec 14, 2008
 *      Author: Ben
 */

#include "guestmodule.h"
#include <QDebug>

GuestModule::GuestModule(QObject *parent)
: QObject(parent)
{
    connect(this, SIGNAL(sendData(QString, QVariant&)), parent, SLOT(dataSender(QString, QVariant&)));
}

GuestModule::~GuestModule() {
}

QString GuestModule::moduleName()
{
	return module;
}

void GuestModule::setModuleName(QString name)
{
    module = name;
}

void GuestModule::receiveData(QVariant data)
{

}

void GuestModule::send(QVariant &data)
{
        emit sendData(module, data);
}
