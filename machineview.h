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
#include "floatingtoolbar.h"
#include "machineconfigobject.h"
#include "machinescrollarea.h"

#include <QScrollArea>
#include <QEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDynamicPropertyChangeEvent>
#include <QSvgWidget>
#include <QShortcut>
#include <QPalette>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class MachineView : public QWidget
{
Q_OBJECT
public:
    explicit MachineView(MachineConfigObject *config, QWidget *parent = 0);

    ~MachineView();

    void showSplash(bool show);
    void captureAllKeys(bool enabled);
    void sendKey(QKeyEvent *event);
    bool event(QEvent * event);
    
public slots:
    void newViewSize();
    void fullscreen(bool enable);
    void initView();

private slots:
    void showToolBar();

signals:
    void fullscreenToggled(bool enabled);

private:

    VncView *view;
    MachineSplash *splash;
    FloatingToolBar *toolBar;
    MachineConfigObject *config;
    QWidget *fullscreenWindow;
    MachineScrollArea *fullscreenScrollArea;
    MachineScrollArea *embeddedScrollArea;
    bool fullscreenEnabled;
    int port;

    //actions... will later be moved to its own file
    QAction *scaleAction;
};

class MinimizePixel : public QWidget
{
    Q_OBJECT
public:
    MinimizePixel(QWidget *parent)
            : QWidget(parent) {
        setFixedSize(1, 1);
        move(QApplication::desktop()->screenGeometry().width() - 1, 0);
    }

signals:
    void rightClicked();

protected:
    void mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::RightButton)
            emit rightClicked();
    }
};


#endif
