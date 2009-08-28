#include "clipboardsync.h"
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QString>
#include <QDebug>

ClipboardSync::ClipboardSync(QObject *parent)
    : GuestModule(parent)
{
    previous = QVariant();
	setModuleName("clipboard");
	clipboard = QApplication::clipboard();
	clipboard->clear(QClipboard::Clipboard);
    clipboard->clear(QClipboard::Selection);
    connect(clipboard, SIGNAL(changed(QClipboard::Mode)), this, SLOT(dataChanged(QClipboard::Mode)));
    qDebug() << "clipboard initialized";
}

ClipboardSync::~ClipboardSync()
{

}

void ClipboardSync::receiveData(QVariant data)
{
    qDebug() << "received clipboard data";
    QVariant::Type type = data.type();
        if(type == QVariant::Image)
	{
                QImage image = data.value<QImage>();
		clipboard->setImage(image, QClipboard::Clipboard);
		clipboard->setImage(image, QClipboard::Selection);
        previous = data;
	}
        else if(type == QVariant::String)
	{
		QString text = data.value<QString>();
		clipboard->setText(text, QClipboard::Clipboard);
		clipboard->setText(text, QClipboard::Selection);
        previous = data;
	}
        else
            qDebug() << "unknown data format!";
}

void ClipboardSync::dataChanged(QClipboard::Mode mode)
{
	if(!clipboard->image(mode).isNull())
	{
		if(previous == QVariant(clipboard->image(mode)))
			return;
		previous = QVariant(clipboard->image(mode));
                send(previous);

	}
	else if(!clipboard->text(mode).isNull())
	{
		if(previous == QVariant(clipboard->text(mode)))
			return;
		previous = QVariant(clipboard->text(mode));
        qDebug() << previous;
                send(previous);
	}
        else
            qDebug() << "got clipboard data, but can't find an image or text!";
}

