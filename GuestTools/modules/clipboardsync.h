#ifndef CLIPBOARDSYNC_H
#define CLIPBOARDSYNC_H

#include "modules/guestmodule.h"
#include <QIODevice>
#include <QClipboard>
#include <QVariant>

class QDataStream;

class ClipboardSync : public GuestModule
{
    Q_OBJECT

public:
    ClipboardSync(QDataStream *stream, QObject *parent = 0);
    ~ClipboardSync();

    virtual void receiveData(QString type, QVariant data);

private:
    QClipboard *clipboard;
    QVariant previous;

public slots:
    void dataChanged(QClipboard::Mode mode);

};

#endif // CLIPBOARDSYNC_H
