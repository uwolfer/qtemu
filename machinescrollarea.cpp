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

#include "machinescrollarea.h"
#include "vnc/vncview.h"
#include <QResizeEvent>
#include <QVariant>

//
MachineScrollArea::MachineScrollArea(QWidget* parent) 
	: QScrollArea(parent)
    , splashShown(false)
{
    setAlignment(Qt::AlignCenter);
    setFrameShape(QFrame::NoFrame);
}

void MachineScrollArea::resizeEvent(QResizeEvent * event)
{
#ifdef DEVELOPER
    qDebug("resized...");
#endif
    resizeView(event->size().width(), event->size().height());
    emit resized(event->size().width(), event->size().height());
    QScrollArea::resizeEvent(event);
}

void MachineScrollArea::resizeView(int widgetWidth, int widgetHeight)
{
    if(splashShown)
    {
        float aspectRatio = (1.0 * widget()->sizeHint().width()/ widget()->sizeHint().height());
        int newWidth = (int)(widgetHeight*aspectRatio);
        int newHeight = (int)(widgetWidth*(1/aspectRatio));

        if(newWidth <= widgetWidth && newHeight > widgetHeight)
            widget()->setFixedSize(newWidth, widgetHeight);
        else
            widget()->setFixedSize(widgetWidth, newHeight);
        return;
    }
 
   widget()->blockSignals(true);
    if(!property("scaleEmbeddedDisplay").toBool())
    {
#ifdef DEVELOPER
        qDebug("no scaling");
#endif
        static_cast<VncView *>(widget())->enableScaling(false);
    }
    else
    {
#ifdef DEVELOPER
        qDebug("scaling");
#endif
       static_cast<VncView *>(widget())->enableScaling(true);
       static_cast<VncView *>(widget())->scaleResize(widgetWidth,widgetHeight);
    }
    widget()->blockSignals(false);
}

bool MachineScrollArea::isSplashShown()
{
    return splashShown;
}

void MachineScrollArea::setSplashShown(bool value)
{
    splashShown = value;
}
//
