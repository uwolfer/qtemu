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

#ifndef HALOBJECT_H
#define HALOBJECT_H

#include <QObject>
#include <QStandardItemModel>
#include <QDBusInterface>
#include <QDBusReply>
#include <QHash>

 struct UsbDevice
 {
     QString id;
     QString vendor;
     QString product;
     QString address;
 };

 struct OptDevice
 {
     QString device;
     QString id;
     QString name;
     QString volume;
     QString volumeId;
 };

class HalObject : public QObject
{
    Q_OBJECT

public:
    HalObject();

    const QStringList deviceList();
    const QList<UsbDevice> usbList();
    const QStringList ifList();
    const QList<OptDevice> opticalList();


private:
    QStringList getDevices();

    QDBusInterface *hal;

    QHash<QString, UsbDevice> usbDeviceHash;
    QHash<QString, OptDevice> optDeviceHash;

signals:
    //generic catchall device notification
    void deviceAdded(const QString devString);
    void deviceRemoved(const QString devString);

    //ethernet notifications
    void ifAdded(const QString ifName, const QString macAddr);
    void ifRemoved(const QString ifName, const QString macAddr);
    //might need if activated/deactivated notifications

    //optical drive notifications
    void opticalAdded(const QString devName, const QString devPath);
    void opticalRemoved(const QString devName, const QString devPath);

    //floppy drive notifications
    void floppyAdded(const QString devName, const QString devPath);
    void floppyRemoved(const QString devName, const QString devPath);

    //usb notifications
    void usbAdded(const QString devString, const UsbDevice device);
    void usbRemoved(const QString devString, const UsbDevice device);

private slots:
    void halDeviceAdded(const QString name);
    void halDeviceRemoved(const QString name);
};

#endif // HALOBJECT_H
