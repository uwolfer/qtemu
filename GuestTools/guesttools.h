#ifndef GUESTTOOLS_H
#define GUESTTOOLS_H

#include <QtGui/QWidget>
#include <QSystemTrayIcon>
#include <QList>
#include <QVariant>
#include "modules/guestmodule.h"
#include "ui_guesttools.h"

 class QAction;
 class QMenu;
 class QextSerialPort;
 class QDataStream;


class GuestTools : public QWidget
{
    Q_OBJECT

public:
    GuestTools(QWidget *parent = 0);
    ~GuestTools();



private:
    Ui::GuestToolsClass ui;

    void createTrayIcon();
    void createActions();

    void createModules();

    void initSerialPort();

    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QextSerialPort *port;
    QDataStream *dataStream;

    QList<GuestModule *> modules;

public slots:
    void dataSender(QString module, QString type, QVariant data);

private slots:
    void clickedIcon(QSystemTrayIcon::ActivationReason reason);

    void ioReceived();
};

#endif // GUESTTOOLS_H
