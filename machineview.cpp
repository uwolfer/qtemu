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

#include "machineview.h"
#include <QUrl>
#include <QSize>

MachineView::MachineView(QWidget *parent, const QString defaultImage)
 : QScrollArea(parent)
{
    scaleable=true;
    if(defaultImage.isEmpty())
        splash = new QSvgWidget("images/oxygen/background.svg", this);
    else
        splash = new QSvgWidget(defaultImage, this);
    view = new VncView();
    showSplash(true);
    this->setWidget(splash);
    this->setAlignment(Qt::AlignCenter);
    this->setFrameShape(QFrame::NoFrame);
    this->setBackgroundRole(QPalette::Window); 
}


MachineView::~MachineView()
{
}

void MachineView::resizeEvent(QResizeEvent * event)
{
#ifdef DEVELOPER
    qDebug("resized...");
#endif
    resizeView(event->size().width(), event->size().height());
    QScrollArea::resizeEvent(event);
}

void MachineView::resizeView(int widgetWidth, int widgetHeight)
{
    if(!scaleable)
    {
        view->setFixedSize(view->framebufferSize().width(), view->framebufferSize().height());
        
        return;
    }

    float aspectRatio = (1.0 * view->framebufferSize().width()) / view->framebufferSize().height();

    int newWidth = widgetHeight*aspectRatio;
    int newHeight = widgetWidth*(1/aspectRatio);

#ifdef DEVELOPER
    qDebug("target aspect ratio: %f",aspectRatio);
#endif

    //if the dimensions for altHeight are better, use them...
    if(newWidth <= widgetWidth && newHeight > widgetHeight)
        view->setFixedSize(newWidth, widgetHeight);
    else
        view->setFixedSize(widgetWidth, newHeight);
}


bool MachineView::isScaleable() const
{
    return scaleable;
}


void MachineView::enableScaling ( bool scale )
{
    scaleable = scale;
    //this->setWidgetResizable(scale);
    newViewSize();
}

void MachineView::machineNumber(int machine)
{
    port = 6900 + machine;
}

void MachineView::initView()
{
    showSplash(true);
    delete view;

    QUrl *url = new QUrl();
    url->setScheme("vnc");
    url->setHost("localhost");
    url->setPort(port);
#ifdef DEVELOPER
    qDebug("connecting to:" + url->toString().toAscii());
#endif
    view = new VncView(this, *url);
    view->start();
    showSplash(false);
    connect(view, SIGNAL(changeSize(int, int)), this, SLOT(newViewSize()));
    
}

void MachineView::showSplash(bool show)
{
   if(!show)
   {
       splash->hide();
       this->takeWidget();
       this->setWidget(view);
       view->show();
   }
   else
   {
       view->hide();
       this->takeWidget();
       this->setWidget(splash);
       splash->show();
   }
}

void MachineView::fullscreen(bool enabled)
{
    view->switchFullscreen(enabled);
}

void MachineView::captureAllKeys(bool enabled)
{
    view->setGrabAllKeys(enabled);
}

void MachineView::sendKey(QKeyEvent * event)
{
    view->keyEvent(event);
}

void MachineView::setSplash(QString location)
{
    splash->load(location);
}

void MachineView::newViewSize()
{
    resizeView(maximumViewportSize().width(), maximumViewportSize().height());
}

