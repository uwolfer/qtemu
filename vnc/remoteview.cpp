/****************************************************************************
**
** Copyright (C) 2002-2003 Tim Jansen <tim@tjansen.de>
** Copyright (C) 2007-2008 Urs Wolfer <uwolfer @ kde.org>
**
** This file is part of KDE.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; see the file COPYING. If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#include "remoteview.h"

#ifndef QTONLY
    #include <KDebug>
    #include <KStandardDirs>
#endif

#include <QBitmap>

RemoteView::RemoteView(QWidget *parent)
        : QWidget(parent),
        m_status(Disconnected),
        m_host(QString()),
        m_port(0),
        m_viewOnly(false),
        m_grabAllKeys(false),
        m_scale(false),
        m_keyboardIsGrabbed(false),
#ifndef QTONLY
        m_wallet(0),
#endif
        m_dotCursorState(CursorOff)
{
}

RemoteView::~RemoteView()
{
#ifndef QTONLY
    delete m_wallet;
#endif
}

RemoteView::RemoteStatus RemoteView::status()
{
    return m_status;
}

void RemoteView::setStatus(RemoteView::RemoteStatus s)
{
    if (m_status == s)
        return;

    if (((1+ m_status) != s) && (s != Disconnected)) {
        // follow state transition rules

        if (s == Disconnecting) {
            if (m_status == Disconnected)
                return;
        } else {
            Q_ASSERT(((int) s) >= 0);
            if (m_status > s) {
                m_status = Disconnected;
                emit statusChanged(Disconnected);
            }
            // smooth state transition
            RemoteStatus origState = m_status;
            for (int i = origState; i < s; i++) {
                m_status = (RemoteStatus) i;
                emit statusChanged((RemoteStatus) i);
            }
        }
    }
    m_status = s;
    emit statusChanged(m_status);
}

bool RemoteView::supportsScaling() const
{
    return false;
}

bool RemoteView::supportsLocalCursor() const
{
    return false;
}

QString RemoteView::host()
{
    return m_host;
}

QSize RemoteView::framebufferSize()
{
    return QSize(0, 0);
}

void RemoteView::startQuitting()
{
}

bool RemoteView::isQuitting()
{
    return false;
}

int RemoteView::port()
{
    return m_port;
}

void RemoteView::updateConfiguration()
{
}

void RemoteView::keyEvent(QKeyEvent *)
{
}

bool RemoteView::viewOnly()
{
    return m_viewOnly;
}

void RemoteView::setViewOnly(bool viewOnly)
{
    m_viewOnly = viewOnly;
}

bool RemoteView::grabAllKeys()
{
    return m_grabAllKeys;
}

void RemoteView::setGrabAllKeys(bool grabAllKeys)
{
    m_grabAllKeys = grabAllKeys;

    if (grabAllKeys) {
        m_keyboardIsGrabbed = true;
        grabKeyboard();
    } else if (m_keyboardIsGrabbed) {
        releaseKeyboard();
    }
}

void RemoteView::showDotCursor(DotCursorState state)
{
    m_dotCursorState = state;
}

RemoteView::DotCursorState RemoteView::dotCursorState() const
{
    return m_dotCursorState;
}

bool RemoteView::scaling() const
{
    return m_scale;
}

void RemoteView::enableScaling(bool scale)
{
    m_scale = scale;
}

void RemoteView::switchFullscreen(bool)
{
}

void RemoteView::scaleResize(int, int)
{
}

KUrl RemoteView::url()
{
    return m_url;
}

#ifndef QTONLY
QString RemoteView::readWalletPassword()
{
    QString krdc_folder = "KRDC";

    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), window()->winId());

    if (m_wallet) {
        bool walletOK = m_wallet->hasFolder(krdc_folder);
        if (!walletOK) {
            walletOK = m_wallet->createFolder(krdc_folder);
            kDebug(5010) << "Wallet folder created";
        }
        if (walletOK) {
            kDebug(5010) << "Wallet OK";
            m_wallet->setFolder(krdc_folder);
            QString password;

            if (m_wallet->hasEntry(m_url.prettyUrl(KUrl::RemoveTrailingSlash)) &&
                    !m_wallet->readPassword(m_url.prettyUrl(KUrl::RemoveTrailingSlash), password)) {
                kDebug(5010) << "Password read OK";

                return password;
            }
        }
    }
    return QString();
}

void RemoteView::saveWalletPassword(const QString &password)
{
    if (m_wallet && m_wallet->isOpen() && !m_wallet->hasEntry(m_url.prettyUrl(KUrl::RemoveTrailingSlash))) {
        kDebug(5010) << "Write wallet password";
        m_wallet->writePassword(m_url.prettyUrl(KUrl::RemoveTrailingSlash), password);
    }
}
#endif

QCursor RemoteView::localDotCursor() const
{
#ifdef QTONLY
    return QCursor(); //TODO
#else
    QBitmap cursorBitmap(KGlobal::dirs()->findResource("appdata",
                                                       "pics/pointcursor.png"));
    QBitmap cursorMask(KGlobal::dirs()->findResource("appdata",
                                                     "pics/pointcursormask.png"));
    return QCursor(cursorBitmap, cursorMask);
#endif
}

void RemoteView::focusInEvent(QFocusEvent *event)
{
    if (m_grabAllKeys) {
        m_keyboardIsGrabbed = true;
        grabKeyboard();
    }

    QWidget::focusInEvent(event);
}

void RemoteView::focusOutEvent(QFocusEvent *event)
{
    if (m_grabAllKeys || m_keyboardIsGrabbed) {
        m_keyboardIsGrabbed = false;
        releaseKeyboard();
    }

    QWidget::focusOutEvent(event);
}

#ifndef QTONLY
#include "moc_remoteview.cpp"
#endif
