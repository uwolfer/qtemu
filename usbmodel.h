#ifndef USBMODEL_H
#define USBMODEL_H

#include <QAbstractTableModel>
#include <QDBusInterface>

class UsbModel : public QObject//public QAbstractTableModel
{
    Q_OBJECT

public:
    UsbModel();

private:
    void getUsbDevices();
   QDBusInterface *hal;
private slots:

};

#endif // USBMODEL_H
