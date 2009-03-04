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
#include <QTimer>
#include <QKeySequence>
#include <QVBoxLayout>


MachineView::MachineView(MachineConfigObject *config, QWidget *parent)
 : QWidget(parent)
    , view(new VncView(this))
    , splash(new MachineSplash(this))
    , config(config)
    , fullscreenEnabled(false)
{
    embeddedScrollArea = new MachineScrollArea(this);
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(embeddedScrollArea);
    setLayout(layout);
     
    showSplash(true);

    connect(embeddedScrollArea, SIGNAL(resized(int, int)), view, SLOT(scaleResize(int, int)));
    

}

MachineView::~MachineView()
{
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
       splash->hide();
       embeddedScrollArea->takeWidget();
       embeddedScrollArea->setWidget(view);
       embeddedScrollArea->setSplashShown(false);
       view->show();

   }
   else
   {
       splash->setPreview(property("preview").toString());
       view->hide();
       embeddedScrollArea->takeWidget();
       embeddedScrollArea->setWidget(splash);
       embeddedScrollArea->setSplashShown(true);
       splash->show();
   }
}

void MachineView::fullscreen(bool enable)
{
    if(enable)
    {
        //entering fullscreen
        showSplash(true);

        fullscreenWindow = new QWidget(this, Qt::Window);
        fullscreenWindow->setWindowTitle(tr("QtEmu Fullscreen") + " (" + property("name").toString() + ")");

        fullscreenScrollArea = new MachineScrollArea(fullscreenWindow);
        fullscreenScrollArea->setWidget(view);
        fullscreenScrollArea->setProperty("scaleEmbeddedDisplay", property("scaleEmbeddedDisplay"));

        connect(fullscreenScrollArea, SIGNAL(resized(int, int)), view, SLOT(scaleResize(int, int)));

        QPalette palette = fullscreenScrollArea->palette();
        palette.setColor(QPalette::Dark, QColor(22,22,22));
        fullscreenScrollArea->setPalette(palette);
        fullscreenScrollArea->setBackgroundRole(QPalette::Dark);

        QVBoxLayout *fullscreenLayout = new QVBoxLayout(fullscreenWindow);
        fullscreenLayout->setMargin(0);
        fullscreenLayout->addWidget(fullscreenScrollArea);

        MinimizePixel *minimizePixel = new MinimizePixel(fullscreenWindow);
        minimizePixel->winId(); // force it to be a native widget (prevents problem with QX11EmbedContainer)
        connect(minimizePixel, SIGNAL(rightClicked()), fullscreenWindow, SLOT(showMinimized()));


        fullscreenWindow->setWindowFlags(Qt::Window);
        fullscreenWindow->showFullScreen();

        showToolBar();

        captureAllKeys(true);
        view->grabKeyboard();

    }
    else if(fullscreenEnabled)
    {
        //exiting fullscreen
        //show();

        fullscreenWindow->setWindowState(0);
        fullscreenWindow->hide();

        

        //get rid of the toolbar
        config->unregisterObject(scaleAction);
        toolBar->hideAndDestroy();
        toolBar->deleteLater();
        toolBar = 0;
        fullscreenWindow->deleteLater();
        fullscreenWindow = 0;
        showSplash(false);
    }
    fullscreenEnabled = enable;
    emit fullscreenToggled(enable);
    view->switchFullscreen(enable);
}

void MachineView::showToolBar()
{
    //create actions
    //TODO: make actions shared between everyplace
    QAction *fullscreenAction = new QAction(QIcon(":/images/oxygen/fullscreen.png"), tr("Fullscreen"), this);
    fullscreenAction->setToolTip(tr("Exit Fullscreen Mode"));
    fullscreenAction->setCheckable(true);
    fullscreenAction->setChecked(true);

    connect(fullscreenAction, SIGNAL(toggled( bool )), this, SLOT(fullscreen(bool)));

    scaleAction = new QAction(QIcon(":/images/oxygen/scale.png"), tr("Scale Display"), this);
    config->registerObject(scaleAction, "scaleEmbeddedDisplay");

    //add a toolbar
    toolBar = new FloatingToolBar(fullscreenWindow, fullscreenWindow);
    toolBar->winId();
    toolBar->setSide(FloatingToolBar::Top);

    toolBar->addAction(fullscreenAction);
    toolBar->addAction(scaleAction);

    QLabel *guestLabel = new QLabel(property("name").toString(), toolBar);
    toolBar->addWidget(guestLabel);
    toolBar->showAndAnimate();
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
    MachineScrollArea *currentScrollArea;
    if(fullscreenEnabled)
        currentScrollArea = fullscreenScrollArea;
    else
        currentScrollArea = embeddedScrollArea;
    
    currentScrollArea->setProperty("scaleEmbeddedDisplay", property("scaleEmbeddedDisplay"));
    currentScrollArea->resizeView(currentScrollArea->maximumViewportSize().width(), currentScrollArea->maximumViewportSize().height());
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
    else if(event->type() == QEvent::Enter&&!embeddedScrollArea->isSplashShown())
    {
        view->setFocus();
        view->grabKeyboard();
        //repainting here fixes an issue where the vncview goes blank on mouseout
        view->repaint();
        return true;
    }
    else if (event->type() == QEvent::Leave)
    {
        view->clearFocus();
        view->releaseKeyboard();
        //repainting here fixes an issue where the vncview goes blank on mouseout
        view->repaint();
        return true;
    }
    else if (event->type() == QEvent::KeyPress) {
         QKeyEvent *ke = static_cast<QKeyEvent *>(event);
         if (ke->key() == Qt::Key_Return && ke->modifiers() == Qt::ControlModifier + Qt::AltModifier) {
             fullscreen(false);
             return true;
         }
    }
    return QWidget::event(event);
}
