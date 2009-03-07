#include "usbmodel.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QStringList>

UsbModel::UsbModel()
{
    qDebug("initializing usb model");
   hal = new QDBusInterface("org.freedesktop.Hal",
                                               "/org/freedesktop/Hal/Manager",
                                               "org.freedesktop.Hal.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
   getUsbDevices();
}

void UsbModel::getUsbDevices()
{
    QDBusReply<QStringList> deviceList = hal->call("GetAllDevices");
   if (deviceList.isValid())
   {
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
               //this is all the info we really need...
               qDebug(
                       tempInterface->call("GetProperty", "usb_device.bus_number").arguments().at(0).toByteArray() + "." +
                       tempInterface->call("GetProperty", "usb_device.linux.device_number").arguments().at(0).toByteArray() + " - " +
                       tempInterface->call("GetProperty", "info.vendor").arguments().at(0).toByteArray() + " : " +
                       tempInterface->call("GetProperty", "info.product").arguments().at(0).toByteArray()
                       );
           }
       }
   }
   else
   {
       //leave the usb model empty
   }
}
