#ifndef CLIPBOARDSYNC_H
#define CLIPBOARDSYNC_H

#include "../guestmodule.h"
#include <QIODevice>
#include <QClipboard>
#include <QVariant>

class QDataStream;

class ClipboardSync : public GuestModule
{
    Q_OBJECT

public:
    ClipboardSync(QObject *parent = 0);
    ~ClipboardSync();

    virtual void receiveData(QVariant data);

private:
    QClipboard *clipboard;
    QVariant previous;

public slots:
    void dataChanged(QClipboard::Mode mode);

};

#endif // CLIPBOARDSYNC_H
