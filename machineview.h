/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop @ gmail.com>
**
** This file is part of QtEmu.
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
** You should have received a copy of the GNU Library General Public License
** along with this library; see the file COPYING.LIB.  If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#ifndef MACHINEVIEW_H
#define MACHINEVIEW_H

#include "vnc/vncview.h"
#include "machinesplash.h"

#include <QScrollArea>
#include <QEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDynamicPropertyChangeEvent>
#include <QSvgWidget>
#include <QShortcut>
#include <QPalette>

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class MachineView : public QScrollArea
{
Q_OBJECT
public:
    MachineView(QWidget *parent = 0);

    ~MachineView();

    void showSplash(bool show);
    void initView();
    void captureAllKeys(bool enabled);
    void sendKey(QKeyEvent *event);
    bool event(QEvent * event);
    
public slots:
    void newViewSize();
    void fullscreen(bool enabled);

private:
    void resizeEvent(QResizeEvent * event);
    void resizeView(int widgetWidth, int widgetHeight);
    void colors();

    QPalette myPalette;
    VncView *view;
    MachineSplash *splash;
    bool splashShown;
    int port;
};



#endif
