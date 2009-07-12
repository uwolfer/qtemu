#include "guesttools.h"
#include "modules/clipboard/clipboardsync.h"
//#include <qextserialport.h>
//#include <QDataStream>
#include <QMenu>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>
#include <QIODevice>

GuestTools::GuestTools(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    blockSize = 0;
    initSerialPort();

    createActions();
    createTrayIcon();
    createModules();

    connect(port, SIGNAL(readyRead()), this, SLOT(ioReceived()));

    trayIcon->show();
}

GuestTools::~GuestTools()
{


}

void GuestTools::createTrayIcon()
 {
    trayIconMenu = new QMenu(this);


    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/guestTray.png"));
    setWindowIcon(QIcon(":/guestTray.png"));
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(clickedIcon(QSystemTrayIcon::ActivationReason)));
 }

void GuestTools::createActions()
{

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));


}

void GuestTools::clickedIcon(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
    {
    	if(this->isVisible())
    	    this->hide();
    	else
            this->show();
    }

}

void GuestTools::ioReceived()
{
    //connect the stream
    QDataStream stream(port);
    stream.setVersion(QDataStream::Qt_4_0);
    //get the size of the data chunk
    if (blockSize == 0) {
        if (port->bytesAvailable() < (int)sizeof(quint64))
            return;
        stream >> blockSize;
    }

    //don't continue until we have all the data
    if ((quint64)(port->bytesAvailable()) < blockSize)
        return;

    QString usesModule;
    QVariant data;


    stream >> usesModule >> data;

    blockSize = 0;

    for(int i = 0; i < modules.size(); i++)
    {
        if(modules.at(i)->moduleName() == usesModule)
        {
            qDebug() << "received data from"<< usesModule;

            modules.at(i)->receiveData(data);
            return;
        }
    }
    qDebug() << "invalid module" << usesModule;
}

void GuestTools::createModules()
{
    modules.append(new ClipboardSync(this));
}

void GuestTools::dataSender(QString module, QVariant &data)
{
    //so that we don't try to send more than one at a time
    //sender()->blockSignals(true);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint64)0;
    out << module;
    out << data;
    out.device()->seek(0);
    out << (quint64)(block.size() - sizeof(quint64));

    port->write(block);

    //re-allow signals
    //sender()->blockSignals(false);

}

void GuestTools::initSerialPort()
{
#ifdef Q_WS_WIN
    port = new QextSerialPort("COM1", QextSerialPort::EventDriven);
#else
    port = new QextSerialPort("ttyS0", QextSerialPort::EventDriven);
#endif
    port->setBaudRate(BAUD115200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_2);
    port->open(QIODevice::ReadWrite);
    if (!(port->lineStatus() & LS_DSR)) {
                qDebug() << "warning: QtEmu not listening";
        }
}
