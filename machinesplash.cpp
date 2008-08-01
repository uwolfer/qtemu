/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop@gmail.com>
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

/****************************************************************************
**
** C++ Implementation: machinesplash
**
** Description: builds the splashscreen each machine has with an embedded
** preview
**
****************************************************************************/

#include "machinesplash.h"
#include <QImage>
#include <QSettings>
#include <QSvgRenderer>
#include <QStackedLayout>
#include <QSvgWidget>
#include <QLabel>
#include <QRectF>
#include <QGridLayout>
#include <QFrame>
#include <QDebug>
#include <QPixmap>
#include <QStyle>

MachineSplash::MachineSplash(QWidget *parent)
 : QWidget(parent)
{
    //set up splash background from splash.svg
    QSettings settings("QtEmu", "QtEmu");
    splashImage = new QSvgWidget(":/images/" + settings.value("iconTheme", "oxygen").toString() + "/splash.svg");
    getPreviewRect();
    previewImage = new QLabel();
    alpha = QPixmap(":/images/" + settings.value("iconTheme", "oxygen").toString() + "/alpha.svg");
    previewImage->setScaledContents(true);
    layout = new QStackedLayout();
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(splashImage);
    layout->addWidget(previewImage);
    setLayout(layout);
    doResize();
    
}


MachineSplash::~MachineSplash()
{
    delete splashImage;
    delete previewImage;
    delete layout;
}

void MachineSplash::setPreview(const QString previewLocation)
{
    if(!previewLocation.isEmpty())
        previewLoc = previewLocation;
    QPixmap preview = QPixmap(previewLoc);
    preview.setAlphaChannel(alpha.scaled(preview.width(),preview.height()));
    previewImage->setPixmap(preview);
    doResize();
}

void MachineSplash::doResize()
{
    getPreviewRect();
    previewImage->setGeometry(previewBounds.toRect());
}

void MachineSplash::resizeEvent(QResizeEvent * event)
{
    doResize();
    QWidget::resizeEvent(event);
}

void MachineSplash::showEvent(QShowEvent * event)
{
    doResize();
    QWidget::showEvent(event);
}


void MachineSplash::getPreviewRect()
{
    QRectF fullsizeBounds = splashImage->renderer()->boundsOnElement("QtEmu_Preview_Screen");
    float scaleFactor = 
        splashImage->width() / splashImage->renderer()->viewBoxF().width();
    previewBounds = QRectF(
        fullsizeBounds.left()*scaleFactor,
        fullsizeBounds.top()*scaleFactor,
        fullsizeBounds.width()*scaleFactor,
        fullsizeBounds.height()*scaleFactor
        );
}


