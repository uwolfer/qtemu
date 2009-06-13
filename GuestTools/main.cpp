

#include <QtGui>
#include <QApplication>
#include <QDebug>

#include "guesttools.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication a(argc, argv);
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
             QMessageBox::critical(0, QObject::tr("Systray"),
                                   QObject::tr("I couldn't detect any system tray "
                                               "on this system."));
             return 1;
         }
         QApplication::setQuitOnLastWindowClosed(false);


    GuestTools w;
    return a.exec();
}
