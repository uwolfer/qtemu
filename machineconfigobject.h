/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop@gmail.com>
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

#ifndef MACHINECONFIGOBJECT_H
#define MACHINECONFIGOBJECT_H

#include <QObject>
#include <QVariant>
#include <QList>
#include "machineconfig.h"


/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/

class MachineConfigObject : public QObject
{
Q_OBJECT
public:
    explicit MachineConfigObject(QObject *parent = 0, MachineConfig *config = 0);

    ~MachineConfigObject();

/**
    add an object to the list along with the config option it uses, stored in the object as a property.
*/
    void registerObject(QObject *object, const QString &nodeType, const QString &nodeName, const QString &optionName = QString(), const QVariant &defaultValue = QVariant());

/**
    registers an object using the short syntax
*/
    void registerObject(QObject *object, const QString &optionName = QString(), const QVariant &defaultValue = QVariant());

/**
    unregister an object
*/
    void unregisterObject(QObject *object);

/**
    event handler for intercepting changes in objects with multiple properties
*/
    bool eventFilter(QObject *object, QEvent *event);

/**
    sets the config object to use (config file object)
*/
    void setConfig(MachineConfig *config);

/**
    gets the config object used (config file object)
*/
    MachineConfig* getConfig();

/**
    get an option from the config file
*/
    QVariant getOption(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &defaultValue);

/**
    get an option from the config file using the short syntax
*/
    QVariant getOption(const QString &optionName, const QVariant &defaultValue = QVariant());

public slots:
/**
    sets an option in the config file
*/
    void setOption(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);

/**
    set an option in the config file using the short syntax
*/
    void setOption(const QString &optionName, const QVariant &value);

/**
    slot is activated if there is a config change
*/
    void configChanged(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);

/**
    get the value of the calling object and sets the value of the associated config option
*/
    void getObjectValue();

private:

    QList<QObject *> registeredObjects;
    MachineConfig *myConfig;


/**
    set an object to the value of its associated config option, or default
*/
    void setObjectValue(QObject *object, const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &defaultValue = QVariant());

};
#endif
