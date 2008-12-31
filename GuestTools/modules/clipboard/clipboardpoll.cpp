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

#include <config-workspace.h>
#include <config-X11.h>

#include "clipboardpoll.h"

#include <kapplication.h>
#include <QClipboard>
#include <kdebug.h>
#include <X11/Xatom.h>
#include <time.h>
#include <QX11Info>

#ifdef HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif

#include "klipper.h"
  
//#define NOISY_KLIPPER_


/*

 The polling magic:

 There's no way with X11 how to find out if the selection has changed (unless its ownership
 is taken away from the current client). In the future, there will be hopefully such notification,
 which will make this whole file more or less obsolete. But for now, Klipper has to poll.
 In order to avoid transferring all the data on every time pulse, this file implements two
 optimizations: The first one is checking whether the selection owner is Qt application (using
 the _QT_SELECTION/CLIPBOARD_SENTINEL atoms on the root window of screen 0), and if yes,
 Klipper can rely on QClipboard's signals. If the owner is not Qt app, and the ownership has changed,
 it means the selection has changed as well. Otherwise, first only the timestamp
 of the last selection change is requested using the TIMESTAMP selection target, and if it's
 the same, it's assumed the contents haven't changed. Note that some applications (like XEmacs) does
 not provide this information, so Klipper has to assume that the clipboard might have changed in this
 case --- this is what is meant by REFUSED below.
 
 Update: Now there's also support for XFixes, so in case XFixes support is detected, only XFixes is
 used for detecting changes, everything else is ignored, even Qt's clipboard signals.

*/

ClipboardPoll::ClipboardPoll()
    : m_xfixes_event_base( -1 )
{
    hide();
    const char* names[ 6 ]
        = { "_QT_SELECTION_SENTINEL",
            "_QT_CLIPBOARD_SENTINEL",
            "CLIPBOARD",
            "TIMESTAMP",
            "KLIPPER_SELECTION_TIMESTAMP",
            "KLIPPER_CLIPBOARD_TIMESTAMP" };
    Atom atoms[ 6 ];
    XInternAtoms( QX11Info::display(), const_cast< char** >( names ), 6, False, atoms );
    m_selection.sentinel_atom = atoms[ 0 ];
    m_clipboard.sentinel_atom = atoms[ 1 ];
    m_xa_clipboard = atoms[ 2 ];
    m_xa_timestamp = atoms[ 3 ];
    m_selection.timestamp_atom = atoms[ 4 ];
    m_clipboard.timestamp_atom = atoms[ 5 ];
    bool use_polling = true;
    kapp->installX11EventFilter( this );
    m_timer.setSingleShot( false );
#ifdef HAVE_XFIXES
    int dummy;
    if( XFixesQueryExtension( QX11Info::display(), &m_xfixes_event_base, &dummy ))
    {
        XFixesSelectSelectionInput( QX11Info::display(), QX11Info::appRootWindow( 0 ), XA_PRIMARY,
            XFixesSetSelectionOwnerNotifyMask |
            XFixesSelectionWindowDestroyNotifyMask |
            XFixesSelectionClientCloseNotifyMask );
        XFixesSelectSelectionInput( QX11Info::display(), QX11Info::appRootWindow( 0 ), m_xa_clipboard,
            XFixesSetSelectionOwnerNotifyMask |
            XFixesSelectionWindowDestroyNotifyMask |
            XFixesSelectionClientCloseNotifyMask );
        use_polling = false;
#ifdef NOISY_KLIPPER_
            kDebug() << "Using XFIXES";
#endif
    }
#endif
    if( use_polling )
        {
#ifdef NOISY_KLIPPER_
        kDebug() << "Using polling";
#endif
        initPolling();
        }
}
    
void ClipboardPoll::initPolling()
{
    connect( kapp->clipboard(), SIGNAL( selectionChanged() ), SLOT(qtSelectionChanged()));
    connect( kapp->clipboard(), SIGNAL( dataChanged() ), SLOT( qtClipboardChanged() ));
    connect( &m_timer, SIGNAL( timeout()), SLOT( timeout()));
    m_timer.start( 1000 );
    m_selection.atom = XA_PRIMARY;
    m_clipboard.atom = m_xa_clipboard;
    m_selection.last_change = m_clipboard.last_change = QX11Info::appTime(); // don't trigger right after startup
    m_selection.last_owner = XGetSelectionOwner( QX11Info::display(), XA_PRIMARY );
#ifdef NOISY_KLIPPER_
    kDebug() << "(1) Setting last_owner for =" << "selection" << ":" << m_selection.last_owner;
#endif
    m_clipboard.last_owner = XGetSelectionOwner( QX11Info::display(), m_xa_clipboard );
#ifdef NOISY_KLIPPER_
    kDebug() << "(2) Setting last_owner for =" << "clipboard" << ":" << m_clipboard.last_owner;
#endif
    m_selection.waiting_for_timestamp = false;
    m_clipboard.waiting_for_timestamp = false;
    updateQtOwnership( m_selection );
    updateQtOwnership( m_clipboard );
}

void ClipboardPoll::qtSelectionChanged()
{
    emit clipboardChanged( true );
}

void ClipboardPoll::qtClipboardChanged()
{
    emit clipboardChanged( false );
}

bool ClipboardPoll::x11Event( XEvent* e )
{
// note that this is also installed as app-wide filter
#ifdef HAVE_XFIXES
    if( m_xfixes_event_base != -1 && e->type == m_xfixes_event_base + XFixesSelectionNotify )
    {
        XFixesSelectionNotifyEvent* ev = reinterpret_cast< XFixesSelectionNotifyEvent* >( e );
        if( ev->selection == XA_PRIMARY && !kapp->clipboard()->ownsSelection())
        {
#ifdef NOISY_KLIPPER_
            kDebug() << "SELECTION CHANGED (XFIXES)";
#endif
            QX11Info::setAppTime( ev->timestamp );
            emit clipboardChanged( true );
        }
        else if( ev->selection == m_xa_clipboard && !kapp->clipboard()->ownsClipboard())
        {
#ifdef NOISY_KLIPPER_
            kDebug() << "CLIPBOARD CHANGED (XFIXES)";
#endif
            QX11Info::setAppTime( ev->timestamp );
            emit clipboardChanged( false );
        }
    }
#endif
    if( e->type == SelectionNotify && e->xselection.requestor == winId())
    {
        if( changedTimestamp( m_selection, *e ) ) {
#ifdef NOISY_KLIPPER_
            kDebug() << "SELECTION CHANGED (GOT TIMESTAMP)";
#endif
            emit clipboardChanged( true );
        }

        if ( changedTimestamp( m_clipboard, *e ) )
        {
#ifdef NOISY_KLIPPER_
            kDebug() << "CLIPBOARD CHANGED (GOT TIMESTAMP)";
#endif
            emit clipboardChanged( false );
        }
        return true; // filter out
    }
    return false;
}

void ClipboardPoll::updateQtOwnership( SelectionData& data )
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long after;
    unsigned char* prop = NULL;
    if( XGetWindowProperty( QX11Info::display(), QX11Info::appRootWindow( 0 ), data.sentinel_atom, 0, 2, False,
        XA_WINDOW, &type, &format, &nitems, &after, &prop ) != Success
        || type != XA_WINDOW || format != 32 || nitems != 2 || prop == NULL )
    {
#ifdef REALLY_NOISY_KLIPPER_
        kDebug() << "UPDATEQT BAD PROPERTY";
#endif
        data.owner_is_qt = false;
        if( prop != NULL )
            XFree( prop );
        return;
    }
    Window owner = reinterpret_cast< long* >( prop )[ 0 ]; // [0] is new owner, [1] is previous
    XFree( prop );
    Window current_owner = XGetSelectionOwner( QX11Info::display(), data.atom );
    data.owner_is_qt = ( owner == current_owner );
#ifdef REALLY_NOISY_KLIPPER_
    kDebug() << "owner=" << owner << "; current_owner=" << current_owner;
    kDebug() << "UPDATEQT:" << ( &data == &m_selection ? "selection" : "clipboard" )  << ":" << data.owner_is_qt;
#endif
}

void ClipboardPoll::timeout()
{
    Klipper::updateTimestamp();
    if( !kapp->clipboard()->ownsSelection() && checkTimestamp( m_selection ) ) {
#ifdef NOISY_KLIPPER_
        kDebug() << "SELECTION CHANGED";
#endif
        emit clipboardChanged( true );
    }
    if( !kapp->clipboard()->ownsClipboard() && checkTimestamp( m_clipboard ) ) {
#ifdef NOISY_KLIPPER_
        kDebug() << "CLIPBOARD CHANGED";
#endif
        emit clipboardChanged( false );
    }

}

bool ClipboardPoll::checkTimestamp( SelectionData& data )
{
    Window current_owner = XGetSelectionOwner( QX11Info::display(), data.atom );
    bool signal = false;
    updateQtOwnership( data );
    if( data.owner_is_qt )
    {
        data.last_change = CurrentTime;
#ifdef REALLY_NOISY_KLIPPER_
        kDebug() << "(3) Setting last_owner for =" << ( &data==&m_selection ?"selection":"clipboard" ) << ":" << current_owner;
#endif
        data.last_owner = current_owner;
        data.waiting_for_timestamp = false;
        return false;
    }
    if( current_owner != data.last_owner )
    {
        signal = true; // owner has changed
        data.last_owner = current_owner;
#ifdef REALLY_NOISY_KLIPPER_
        kDebug() << "(4) Setting last_owner for =" << ( &data==&m_selection ?"selection":"clipboard" ) << ":" << current_owner;
#endif
        data.waiting_for_timestamp = false;
        data.last_change = CurrentTime;
#ifdef REALLY_NOISY_KLIPPER_
        kDebug() << "OWNER CHANGE:" << ( data.atom == XA_PRIMARY ) << ":" << current_owner;
#endif
        return true;
    }
    if( current_owner == None ) {
        return false; // None also last_owner...
    }
    if( data.waiting_for_timestamp ) {
        // We're already waiting for the timestamp of the last check
        return false;
    }
    XDeleteProperty( QX11Info::display(), winId(), data.timestamp_atom );
    XConvertSelection( QX11Info::display(), data.atom, m_xa_timestamp, data.timestamp_atom, winId(), QX11Info::appTime() );
    data.waiting_for_timestamp = true;
    data.waiting_x_time = QX11Info::appTime();
#ifdef REALLY_NOISY_KLIPPER_
    kDebug() << "WAITING TIMESTAMP:" << ( data.atom == XA_PRIMARY );
#endif
    return false;
}

bool ClipboardPoll::changedTimestamp( SelectionData& data, const XEvent& ev )
{
    if( ev.xselection.requestor != winId()
        || ev.xselection.selection != data.atom
        || ev.xselection.time != data.waiting_x_time )
    {
        return false;
    }
    data.waiting_for_timestamp = false;
    if( ev.xselection.property == None )
    {
#ifdef NOISY_KLIPPER_
        kDebug() << "REFUSED:" << ( data.atom == XA_PRIMARY );
#endif
        return true;
    }
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long after;
    unsigned char* prop = NULL;
    if( XGetWindowProperty( QX11Info::display(), winId(), ev.xselection.property, 0, 1, False,
        AnyPropertyType, &type, &format, &nitems, &after, &prop ) != Success
        || format != 32 || nitems != 1 || prop == NULL )
    {
#ifdef NOISY_KLIPPER_
        kDebug() << "BAD PROPERTY:" << ( data.atom == XA_PRIMARY );
#endif
        if( prop != NULL )
            XFree( prop );
        return true;
    }
    Time timestamp = reinterpret_cast< long* >( prop )[ 0 ];
    XFree( prop );
#ifdef NOISY_KLIPPER_
    kDebug() << "GOT TIMESTAMP:" << ( data.atom == XA_PRIMARY );
    kDebug() <<   "timestamp=" << timestamp
              << "; CurrentTime=" << CurrentTime
              << "; last_change=" << data.last_change
              << endl;
#endif
    if( timestamp != data.last_change || timestamp == CurrentTime )
    {
#ifdef NOISY_KLIPPER_
        kDebug() << "TIMESTAMP CHANGE:" << ( data.atom == XA_PRIMARY );
#endif
        data.last_change = timestamp;
        return true;
    }
    return false; // ok, same timestamp
}

#include "clipboardpoll.moc"
