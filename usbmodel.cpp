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
#include "halobject.h"
#include "qtemuenvironment.h"
#include <QStringList>
#include <QStandardItem>

UsbModel::UsbModel(MachineConfigObject * config, QObject * parent)
        :QStandardItemModel(parent)
        ,config(config)
{
    hal = QtEmuEnvironment::getHal();

    getUsbDevices();

    loadConfig();
    connect(this, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(getChange(QStandardItem*)));
    connect(hal, SIGNAL(usbAdded(QString,UsbDevice)), this, SLOT(deviceAdded(QString,UsbDevice)));
    connect(hal, SIGNAL(usbRemoved(QString,UsbDevice)), this, SLOT(deviceRemoved(QString,UsbDevice)));
}

void UsbModel::getUsbDevices()
{

    QList<UsbDevice> deviceList = hal->usbList();

    if (!deviceList.isEmpty())
    {
        clear();

        for(int i = 0; i < deviceList.size(); i++)
        {
            addItem(deviceList.at(i).vendor + " - " + deviceList.at(i).product, deviceList.at(i).id);
        }
    }
}

void UsbModel::addItem(const QString deviceName, QString id)
{
#ifdef DEVELOPER
    qDebug(QString(deviceName + ' ' + id).toAscii());
#endif
    QList<QStandardItem*> items;
    items.append(new QStandardItem(deviceName));
    id.remove("/org/freedesktop/Hal/devices/usb_device_");
    items.append(new QStandardItem(id));
    items.at(0)->setCheckable(true);
    appendRow(items);

    //appendRow(QStandardItem(deviceName).setData(id));
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
    QString address;

    //find the next free name for a usb host device
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
                address = hal->usbList().at(i).address;

                bool checkExists = false;
                for(int j = 0; j< names.size(); j++)
                {
                    if(config->getOption("usb", names.at(j), "id", QString()) == item(i,1)->text())
                    {
                        config->setOption("usb", names.at(j), "address", address);
                        emit(vmDeviceAdded(address));
                        checkExists = true;
                        break;
                    }
                }
                if(!checkExists)
                {
                    config->setOption("usb", nextFreeName, "id", item(i,1)->text());
                    config->setOption("usb", nextFreeName, "address", address);
                    emit(vmDeviceAdded(address));
                }
            }
            else
            {
                for(int j = 0; j < names.size(); j++)
                {
                    if(config->getOption("usb", names.at(j), "id", QString()) == item(i,1)->text())
                    {
                        emit(vmDeviceRemoved(config->getOption("usb", names.at(j), "address", "0.0").toString()));
                        config->getConfig()->clearOption("usb", names.at(j), "id");
                        config->getConfig()->clearOption("usb", names.at(j), "address");
                        config->getConfig()->clearOption("usb","",names.at(j));
                    }
                }

            }
            break;
        }

    }



}

void UsbModel::deviceAdded(QString name, UsbDevice device)
{
#ifdef DEVELOPER
    qDebug(QString("device added " + device.vendor + '-' + device.product + device.id).toAscii());
#endif
    QString id = device.id;
    addItem(device.vendor + " - " + device.product, id.remove("/org/freedesktop/Hal/devices/usb_device_"));
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

void UsbModel::deviceRemoved(QString name, UsbDevice device)
{
    for(int i=0;i<this->rowCount(QModelIndex());i++)
    {
        if((QString("/org/freedesktop/Hal/devices/usb_device_") + QString(item(i,1)->text())) == name)
        {
#ifdef DEVELOPER
            qDebug("device removed " + name.toAscii());
#endif
            this->removeRow(i);
            //getUsbDevices();
            loadConfig();
            break;
        }
    }
}
