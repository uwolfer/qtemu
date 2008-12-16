#include "clipboardsync.h"
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QString>
#include <QDebug>

ClipboardSync::ClipboardSync(QDataStream *stream, QObject *parent)
    : GuestModule(stream, parent)
{
	setModuleName("clipboard");
	clipboard = QApplication::clipboard();
	clipboard->clear(QClipboard::Clipboard);
    connect(clipboard, SIGNAL(changed(QClipboard::Mode)), this, SLOT(dataChanged(QClipboard::Mode)));
}

ClipboardSync::~ClipboardSync()
{

}

void ClipboardSync::receiveData(QString type, QVariant data)
{
	if(type == "QImage")
	{
		QImage image = data.value<QImage>();
		clipboard->setImage(image, QClipboard::Clipboard);
		clipboard->setImage(image, QClipboard::Selection);
	}
	else if(type == "QString")
	{
		QString text = data.value<QString>();
		clipboard->setText(text, QClipboard::Clipboard);
		clipboard->setText(text, QClipboard::Selection);
	}
}

void ClipboardSync::dataChanged(QClipboard::Mode mode)
{
	if(!clipboard->image(mode).isNull())
	{
		//emit sendData( "clipboard", "QString", QVariant(QVariant::fromValue<QImage>(clipboard->image(mode))));
		//*stream << "QImage" << QImage(clipboard->image(mode));
		if(previous == QVariant(clipboard->image(mode)))
			return;
		previous = QVariant(clipboard->image(mode));
		send("QImage", previous);

	}
	else if(!clipboard->text(mode).isNull())
	{
		if(previous == QVariant(clipboard->text(mode)))
			return;
		previous = QVariant(clipboard->text(mode));
		send("QString", previous);
	}
}

