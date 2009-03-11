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

#include "usbmodel.h"
#include "machineconfigobject.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QStringList>
#include <QStandardItem>

UsbModel::UsbModel(MachineConfigObject * config, QObject * parent)
        :QStandardItemModel(parent)
        ,config(config)
{
   hal = new QDBusInterface("org.freedesktop.Hal",
                                               "/org/freedesktop/Hal/Manager",
                                               "org.freedesktop.Hal.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
   getUsbDevices();

   loadConfig();
   connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(getChange(QStandardItem*)));
   connect(hal, SIGNAL(DeviceAdded(QString)), this, SLOT(deviceAdded(QString)));
   connect(hal, SIGNAL(DeviceRemoved(QString)), this, SLOT(deviceRemoved(QString)));
}

void UsbModel::getUsbDevices()
{
    QDBusReply<QStringList> deviceList = hal->call("GetAllDevices");
   if (deviceList.isValid())
    {
       clear();
       //search through and get those items that are USB and not hubs.
       QDBusInterface *tempInterface;
       for(int i = 0; i < deviceList.value().size(); i++)
       {
           tempInterface = new QDBusInterface("org.freedesktop.Hal",
                                               deviceList.value().at(i),
                                               "org.freedesktop.Hal.Device",
                                               QDBusConnection::systemBus(),
                                               this);
           if(tempInterface->call("GetProperty", "info.subsystem").arguments().at(0).toString() == "usb_device" && tempInterface->call("GetProperty", "usb_device.num_ports").arguments().at(0).toInt() == 0 )
           {
#ifdef DEVELOPER
               qDebug(
                       tempInterface->call("GetProperty", "usb_device.bus_number").arguments().at(0).toByteArray() + "." +
                       tempInterface->call("GetProperty", "usb_device.linux.device_number").arguments().at(0).toByteArray() + " - " +
                       tempInterface->call("GetProperty", "info.vendor").arguments().at(0).toByteArray() + " : " +
                       tempInterface->call("GetProperty", "info.product").arguments().at(0).toByteArray()
                       );
#endif
               QList<QStandardItem*> items;
               items.append(new QStandardItem(tempInterface->call("GetProperty", "info.vendor").arguments().at(0).toString() + " - " + tempInterface->call("GetProperty", "info.product").arguments().at(0).toString()));
               QString id = deviceList.value().at(i);
               items.append(new QStandardItem(id.remove("/org/freedesktop/Hal/devices/usb_device_")));
               items.at(0)->setCheckable(true);
               appendRow(items);


           }
       }
   }
}

void UsbModel::loadConfig()
{
    QStringList names = config->getConfig()->getAllOptionNames("usb", "");
    for(int i=0;i<names.size();i++)
    {
        if(names.at(i).contains("host"))
            checkDevice(config->getOption("usb", names.at(i),"id",QString()).toString());
    }
}

void UsbModel::checkDevice(QString deviceName)
{
    for(int i = 0; i < rowCount(QModelIndex()); i++)
    {
        if(item(i,1)->text() == deviceName)
        {
            item(i,0)->setCheckState(Qt::Checked);
            break;
        }
    }
}

void UsbModel::getChange(QStandardItem * thisItem)
{
    QStringList names = config->getConfig()->getAllOptionNames("usb", "");
    QString nextFreeName;
    for(int i = 0; i < names.size() + 1;i++)
    {
        if(!names.contains("host" + QString::number(i)))
        {
            nextFreeName = "host" + QString::number(i);
        }
    }

    for(int i = 0; i < rowCount(QModelIndex()); i++)
    {
        if(item(i,0)->text() == thisItem->text())
        {
            if(thisItem->checkState() == Qt::Checked)
            {
                bool checkExists = false;
                for(int j = 0; j< names.size(); j++)
                {
                    if(config->getOption("usb", names.at(j), "id", QString()) == item(i,1)->text())
                    {
                        checkExists = true;
                        break;
                    }
                }
                if(!checkExists)
                    config->setOption("usb", nextFreeName, "id", item(i,1)->text());
            }
            else
            {
                for(int j = 0; j < names.size(); j++)
                {
                    if(config->getOption("usb", names.at(j), "id", QString()) == item(i,1)->text())
                    {
                        config->getConfig()->clearOption("usb", names.at(j), "id");
                        config->getConfig()->clearOption("usb","",names.at(j));
                    }
                }
            }
            break;
        }

    }



}

void UsbModel::deviceAdded(QString name)
{
    QDBusInterface * tempInterface = new QDBusInterface("org.freedesktop.Hal",
                                               name,
                                               "org.freedesktop.Hal.Device",
                                               QDBusConnection::systemBus(),
                                               this);
    if(tempInterface->call("GetProperty", "info.subsystem").arguments().at(0).toString() == "usb_device" && tempInterface->call("GetProperty", "usb_device.num_ports").arguments().at(0).toInt() == 0 )
    {
#ifdef DEVELOPER
        qDebug("device added " + name.toAscii());
#endif
        getUsbDevices();
        loadConfig();
        if(property("autoAddDevices").toBool())
        {
            for(int i=0;i<this->rowCount(QModelIndex());i++)
            {
                if((QString("/org/freedesktop/Hal/devices/usb_device_") + QString(item(i,1)->text())) == name)
                {
                    item(i,0)->setCheckState(Qt::Checked);
                }
            }
        }
    }

}

void UsbModel::deviceRemoved(QString name)
{
    for(int i=0;i<this->rowCount(QModelIndex());i++)
    {
        if((QString("/org/freedesktop/Hal/devices/usb_device_") + QString(item(i,1)->text())) == name)
        {
#ifdef DEVELOPER
            qDebug("device removed " + name.toAscii());
#endif
            getUsbDevices();
            loadConfig();
            break;
        }
    }
}
