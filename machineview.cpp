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
#include <QColor>
#include <QLabel>
#include <QAction>


MachineView::MachineView(QWidget *parent)
 : QScrollArea(parent)
    , view(new VncView(this))
    , splash(new MachineSplash(this))
{
    showSplash(true);
    setAlignment(Qt::AlignCenter);
    setFrameShape(QFrame::NoFrame);
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
    if(splashShown)
    {
        float aspectRatio = (1.0 * splash->sizeHint().width()/ splash->sizeHint().height());
        int newWidth = widgetHeight*aspectRatio;
        int newHeight = widgetWidth*(1/aspectRatio);

        if(newWidth <= widgetWidth && newHeight > widgetHeight)
            widget()->setFixedSize(newWidth, widgetHeight);
        else
            widget()->setFixedSize(widgetWidth, newHeight);
        return;
    }
 
   view->blockSignals(true);
    if(!property("scaleEmbeddedDisplay").toBool())
    {
        qDebug("no scaling");
        view->enableScaling(false);
    }
    else
    {
        qDebug("scaling");
       view->enableScaling(true);
       view->scaleResize(widgetWidth,widgetHeight);
    }
    view->blockSignals(false);
}

void MachineView::initView()
{
    showSplash(true);
    delete view;

    QUrl url;
    url.setScheme("vnc");
    url.setHost(property("vncHost").toString());
    url.setPort(property("vncPort").toInt() + 5900);

//#ifdef DEVELOPER
    qDebug("connecting to:" + url.toString().toAscii());
//#endif
    view = new VncView(this, url);
    view->start();
    showSplash(false);
    connect(view, SIGNAL(changeSize(int, int)), this, SLOT(newViewSize()));

}

void MachineView::showSplash(bool show)
{
   if(!show)
   {
       //initView();
       splash->hide();
       takeWidget();
       setWidget(view);
       splashShown = false;
       view->show();

   }
   else
   {
       fullscreen(false);
       splash->setPreview(property("preview").toString());
       view->hide();
       takeWidget();
       setWidget(splash);
       splashShown = true;
       splash->show();
   }
}

void MachineView::fullscreen(bool enabled)
{
    if(enabled)
    {
        if(splashShown)
        {
            fullscreen(false);
            return;
        }
        setWindowFlags(Qt::Window);
        showFullScreen();
        QPalette p;
        p.setColor(QPalette::Background, QColor(22,22,22));
        setPalette(p);

        //add a toolbar
        toolBar = new FloatingToolBar(this,this);
        toolBar->setSide(FloatingToolBar::Top);
        QLabel *hostLabel = new QLabel("fullscreen", toolBar);
        toolBar->addWidget(hostLabel);
        toolBar->addAction(new QAction("disable fullscreen mode", this));
//        toolBar->setSticky(true);
        toolBar->showAndAnimate();

    }
    else
    {
        setWindowFlags(Qt::Widget);
        showNormal();
        setPalette(QPalette());
    }
    emit fullscreenToggled(enabled);
    show();
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

void MachineView::newViewSize()
{
    resizeView(maximumViewportSize().width(), maximumViewportSize().height());
}

bool MachineView::event(QEvent * event)
{
    if(event->type() == QEvent::DynamicPropertyChange)
    {
        //any property changes dealt with in here
        QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
        if(propEvent->propertyName() == "scaleEmbeddedDisplay")
        {
            newViewSize();
        }
        else if(propEvent->propertyName() == "preview")
        {
            splash->setPreview(property("preview").toString());

        }
        return false;
    }
    else if(event->type() == QEvent::Enter&&!splashShown)
    {
        view->setFocus();
        view->grabKeyboard();
        return true;
    }
    else if (event->type() == QEvent::Leave)
    {
        view->clearFocus();
        view->releaseKeyboard();
        return true;
    }
    else if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ke = static_cast<QKeyEvent *>(event);
         if (ke->key() == Qt::Key_Return && ke->modifiers() == Qt::ControlModifier + Qt::AltModifier) {
             fullscreen(false);
             return true;
         }
    }
    return QScrollArea::event(event);
}
