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
** C++ Implementation: ConfigEvent
**
** Description: 
**
****************************************************************************/

#include "configevent.h"

ConfigEvent::ConfigEvent(QObject *parent)
 : QObject(parent)
{
}

ConfigEvent::ConfigEvent(const QString nodeType, const QString nodeName, const QString optionName, const QVariant value, const QVariant oldValue, QObject * parent)
 : QObject(parent)
{
    this->nodeType = nodeType;
    this->nodeName = nodeName;
    this->optionName = optionName;
    this->value = value;
    this->oldValue = oldValue;

}
ConfigEvent::ConfigEvent(const ConfigEvent & configEvent, QObject * parent)
 : QObject(parent)
{
ConfigEvent::ConfigEvent(configEvent.nodeType, configEvent.nodeName, configEvent.optionName, configEvent.value, configEvent.oldValue, parent);
}

ConfigEvent::~ConfigEvent()
{
}

