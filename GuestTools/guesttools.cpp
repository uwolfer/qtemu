#include "guesttools.h"
#include "modules/clipboardsync.h"
#include <qextserialport.h>
#include <QDataStream>
#include <QMenu>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>
#include <QIODevice>

GuestTools::GuestTools(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

    initSerialPort();

	dataStream = new QDataStream(port);

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

	QString usesModule;
	QString type;
	QVariant data;
	*dataStream >> usesModule;
	*dataStream >> type;
	*dataStream >> data;
    for(int i = 0; i < modules.size(); i++)
    {
    	if(modules.at(i)->moduleName() == usesModule)
    	{
    		modules.at(i)->receiveData(type, data);
    	}
    }
}

void GuestTools::createModules()
{
    modules.append(new ClipboardSync(dataStream, this));
}

void GuestTools::dataSender(QString module, QString type, QVariant data)
{
    sender()->blockSignals(true);
	*dataStream << module << type << data;
	sender()->blockSignals(false);

}

void GuestTools::initSerialPort()
{
	//modify the port settings on your own
	#ifdef _TTY_POSIX_
		port = new QextSerialPort("/dev/ttyS0", QextSerialPort::Polling);
	#else
		port = new QextSerialPort("COM1", QextSerialPort::EventDriven);
	#endif /*_TTY_POSIX*/
	port->setBaudRate(BAUD19200);
	port->setFlowControl(FLOW_OFF);
	port->setParity(PAR_NONE);
	port->setDataBits(DATA_8);
	port->setStopBits(STOP_2);
	//set timeouts to 500 ms
	port->setTimeout(500);


	port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}
