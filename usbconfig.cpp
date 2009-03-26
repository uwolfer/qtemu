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

#include "usbconfig.h"
#include "machineconfigobject.h"
#include "machineprocess.h"
#include "machinetab.h"

UsbConfig::UsbConfig(MachineProcess * parent, MachineConfigObject * config)
        : QObject(parent)
        , config(config)
        , parent(parent)
{
    config->registerObject(this);

    qobject_cast<MachineTab *>(parent->parent());
}

QStringList UsbConfig::getOptionString()
{
    QStringList hostDeviceNames = config->getConfig()->getAllOptionNames("usb", "");
    QStringList optionString;
    foreach(QString name, hostDeviceNames)
    {
        optionString << "-usbdevice" << "host:" + config->getOption("usb", name, "address", QString()).toString();
    }
    return optionString;
}

void UsbConfig::vmAddDevice(QString id)
{
    if(parent->state() == 2)
    {
        parent->write(QString("usb_add host:" + id).toAscii());
    }
}

void UsbConfig::vmRemoveDevice(QString id)
{    if(parent->state() == 2)
    {
        parent->write(QString("usb_del host:" + id).toAscii());
    }

    //this may not be right... or needed sometimes.
    //if the device is physically removed this is exteranious,
    //if you just uncheck the device's checkbox, then it is required.
}
