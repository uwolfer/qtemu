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

#ifndef USBMODEL_H
#define USBMODEL_H

#include <QStandardItemModel>
#include "halobject.h"

class MachineConfigObject;

class UsbModel : public QStandardItemModel
{
    Q_OBJECT

public:
    UsbModel(MachineConfigObject * config, QObject * parent);

private:
    void getUsbDevices();
    void loadConfig();
    void checkDevice(QString deviceName);
    void addItem(QString deviceName, QString id);

    MachineConfigObject *config;
    HalObject *hal;

private slots:
    void getChange(QStandardItem * item);
    void deviceAdded(QString name, UsbDevice device);
    void deviceRemoved(QString name, UsbDevice device);

signals:
    void vmDeviceAdded(QString identifier);
    void vmDeviceRemoved(QString identifier);
};

#endif // USBMODEL_H
