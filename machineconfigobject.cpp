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

/****************************************************************************
**
** C++ Implementation: machineconfigobject
**
** Description: implements an event to signal config changes and 
** a way to save and retrieve config values.
**
****************************************************************************/

#include "machineconfigobject.h"
#include <QObject>
#include <QStringList>
#include <QDynamicPropertyChangeEvent>
#include <QEvent>
#include <QButtonGroup>
#include <QAbstractButton>

MachineConfigObject::MachineConfigObject(QObject *parent, MachineConfig *config)
 : QObject(parent)
{
    setConfig(config);
    connect(config, SIGNAL(optionChanged(QString, QString, QString, QVariant)),this,SLOT(configChanged(QString, QString, QString, QVariant)));
}


MachineConfigObject::~MachineConfigObject()
{
}

/**
    sets the config object to use (config file object)
*/
void MachineConfigObject::setConfig(MachineConfig * config)
{
    if(config!=0)
        myConfig = config;
    else
       myConfig = new MachineConfig();
}

/**
    get an option from the config file
*/
QVariant MachineConfigObject::getOption(const QString nodeType, const QString nodeName, const QString optionName, const QVariant defaultValue)
{
    return myConfig->getOption(nodeType, nodeName, optionName, defaultValue);
}

/**
    get an option from the config file using the short syntax
*/
QVariant MachineConfigObject::getOption(const QString optionName, const QVariant defaultValue)
{
    return getOption("machine", QString(), optionName, defaultValue);
}

/**
    sets an option in the config file
*/
void MachineConfigObject::setOption(const QString nodeType, const QString nodeName, const QString optionName, const QVariant value)
{
    myConfig->setOption(nodeType, nodeName, optionName, value);
}

/**
    set an option in the config file using the short syntax
*/
void MachineConfigObject::setOption(const QString optionName, const QVariant value)
{
    setOption("machine", QString(), optionName, value);
}

/**
    add an object to the list along with the config option it uses, stored in the object as a property.
*/
void MachineConfigObject::registerObject(QObject *object, const QString nodeType, const QString nodeName, const QString optionName, const QVariant defaultValue)
{

    object->setProperty("nodeType", nodeType);
    object->setProperty("nodeName", nodeName);

    if(optionName.isEmpty())
    {
        //dealing with an object with multiple properties
        //add all properties in the config under this node name/type
        QStringList options = myConfig->getAllOptionNames(nodeType, nodeName);
        for(int i=0;i<options.size();i++)
        {
            object->setProperty(options.at(i).toAscii(), myConfig->getOption(nodeType, nodeName, options.at(i)));
        }

        //add event filter
        object->installEventFilter(this);
    }
    else
    {
        //object with one property
        object->setProperty("optionName", optionName);

        //set object value
        setObjectValue(object, nodeType, nodeName, optionName, defaultValue);
        //connect signals/slots - based on type
    }
    registeredObjects.append(object);
}

/**
    registers an object using the short syntax
*/
void MachineConfigObject::registerObject(QObject *object, const QString optionName, const QVariant defaultValue)
{
    registerObject(object, "machine", QString(), optionName, defaultValue);
}

/**
    unregister an object
*/
void MachineConfigObject::unregisterObject(QObject *object)
{
    registeredObjects.removeAll(object);
}

/**
    set an object's property to the value of its associated config option, or default
*/
void MachineConfigObject::setObjectValue(QObject * object, const QString nodeType, const QString nodeName, const QString optionName, const QVariant defaultValue)
{


    //get the value from the config
    QVariant value = getOption(nodeType, nodeName, optionName, defaultValue);
    //qDebug("setting object for " + optionName.toAscii() + " to " + value.toByteArray());
    //disconnect so that we don't go in a loop forever

    //set the object's value, analyzing its type / properties to determine how to do so.
    if(object->property("optionName").isNull())
    {
        object->removeEventFilter(this);
        object->setProperty(optionName.toAscii(), value);
        object->installEventFilter(this);
    }
    //QButtonGroup handling is sticky...
    else if(object->inherits("QButtonGroup"))
    {
        object->disconnect(this);
        QButtonGroup *group = static_cast<QButtonGroup *>(object);
        QList<QAbstractButton *> buttons = group->buttons();
        for(int i=0;i<buttons.size();i++)
        {
            if(buttons.at(i)->text() == value.toString())
            {
                buttons.at(i)->setProperty("checked", true);
            }
            else
            {
                buttons.at(i)->setProperty("checked", false);
            }
        }
        connect(object, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(getObjectValue()));
    }
    else if (object->inherits("QAbstractButton"))
    {
        object->disconnect(this);
        object->setProperty("checked", value);
        connect(object, SIGNAL(toggled(bool)), this, SLOT(getObjectValue()));
    }
    else if (object->inherits("QSpinBox")||object->inherits("QAbstractSlider"))
    {
        object->disconnect(this);
        object->setProperty("value", value);
        connect(object, SIGNAL(valueChanged(int)), this, SLOT(getObjectValue()));
    }
    else if (object->inherits("QLineEdit"))
    {
        object->disconnect(this);
        object->setProperty("text", value);
        connect(object, SIGNAL(textChanged(QString)), this, SLOT(getObjectValue()));
    }
    else if (object->inherits("QTextEdit"))
    {
        object->disconnect(this);
        object->setProperty("plainText", value);
        connect(object, SIGNAL(textChanged()), this, SLOT(getObjectValue()));
    }
    else
    {
        //if it's none of those... we don't know what it is yet.
        qDebug("unknown object type" + QByteArray(object->metaObject()->className()));
    }
    //qDebug("set!");
}

/**
    event handler for intercepting changes in objects with multiple properties
*/
bool MachineConfigObject::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *myEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
        QString nodeType;
        QString nodeName;
        QString optionName;
        QVariant value;
        nodeType = object->property("nodeType").toString();
        nodeName = object->property("nodeName").toString();
        optionName = myEvent->propertyName();
        value = object->property(optionName.toAscii());
        //save the option to the config
        setOption(nodeType, nodeName, optionName, value);
    }
    // allow further processing
    return false;
}

/**
    get the value of the calling object and sets the value of the associated config option
*/
void MachineConfigObject::getObjectValue()
{
    QVariant value;
    QObject *object = sender();
    //get value from the object, analyzing its type / properties to determine how to do so.
    if (object->inherits("QButtonGroup"))
    {
        QButtonGroup *group = static_cast<QButtonGroup *>(object);
        value = group->checkedButton()->text();
    }
    else if (object->inherits("QAbstractButton"))
    {
        value = object->property("checked");
    }
    else if (object->inherits("QSpinBox")||object->inherits("QAbstractSlider"))
    {
        value = object->property("value");
    }
    else if (object->inherits("QLineEdit"))
    {
        value = object->property("text");
    }
    else if (object->inherits("QTextEdit"))
    {
        value = object->property("plainText");
    }
    else
    {
        //if it's none of those... we don't know what it is yet.
        qDebug("unknown object type" + QByteArray(object->metaObject()->className()));
    }

    //save the option to the config
    if(value.isValid())
    {
        setOption(object->property("nodeType").toString(), object->property("nodeName").toString(), object->property("optionName").toString(), value);
    }
}


/**
    slot is activated if there is a config change
*/
void MachineConfigObject::configChanged(const QString nodeType, const QString nodeName, const QString optionName, const QVariant value)
{
    QObject *object;
    for(int i=0;i<registeredObjects.size();i++)
    {
        object = registeredObjects.at(i);
        QString thisNodeType = object->property("nodeType").toString();
        QString thisNodeName = object->property("nodeName").toString();
        QString thisOptionName = object->property("optionName").toString();
        if(( thisNodeType == nodeType ) && ( thisNodeName == nodeName || thisNodeName.isEmpty() ) && ( thisOptionName.isEmpty() || thisOptionName == optionName ))
        {
                setObjectValue(object, nodeType, nodeName, optionName, value);
        }
    }

}
