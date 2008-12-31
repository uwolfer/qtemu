// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2003 by Lubos Lunak <l.lunak@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _CLIPBOARDPOLL_H_
#define _CLIPBOARDPOLL_H_

#include <QWidget>
#include <QTimer>
#include <X11/Xlib.h>
#include <fixx11h.h>

class ClipboardPoll
    : public QWidget
    {
    Q_OBJECT
    public:
        ClipboardPoll();
    Q_SIGNALS:
        void clipboardChanged( bool selectionMode );
    protected:
        virtual bool x11Event( XEvent* );
    private Q_SLOTS:
        void timeout();
        void qtSelectionChanged();
        void qtClipboardChanged();
    private:
        struct SelectionData
        {
            Atom atom;
            Atom sentinel_atom;
            Atom timestamp_atom;
            Window last_owner;
            bool owner_is_qt;
            Time last_change;
            bool waiting_for_timestamp;
            Time waiting_x_time;
        };
        void updateQtOwnership( SelectionData& data );
        bool checkTimestamp( SelectionData& data );
        bool changedTimestamp( SelectionData& data, const XEvent& e );
        void initPolling();
        QTimer m_timer;
        SelectionData m_selection;
        SelectionData m_clipboard;
        Atom m_xa_clipboard;
        Atom m_xa_timestamp;
        int m_xfixes_event_base;
    };
    
#endif
