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
** Description: 
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

MachineSplash::MachineSplash(QWidget *parent)
 : QWidget(parent)
{
    //set up splash background from splash.svg
    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();
    splashImage = new QSvgWidget(":/images/" + iconTheme + "/splash.svg");
    getPreviewRect();
    previewImage = new QLabel();
    previewImage->setScaledContents(true);
    layout = new QStackedLayout();
    previewLayout = new QGridLayout();
    QFrame *previewFrame = new QFrame();
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(splashImage);
    previewFrame->setLayout(previewLayout);
    layout->addWidget(previewFrame);
    previewLayout->addWidget(previewImage, 1, 1);
    previewLayout->setColumnStretch(1, 10);
    previewLayout->setRowStretch(1, 10);
    setLayout(layout);
    
}


MachineSplash::~MachineSplash()
{
}

void MachineSplash::setPreview(const QString previewLocation)
{
    previewImage->setPixmap(QPixmap(previewLocation));
    doResize();
}

void MachineSplash::doResize()
{
    getPreviewRect();
    previewImage->setFixedSize(previewBounds->width(), previewBounds->height());
    previewLayout->setRowMinimumHeight(0, previewBounds->top() - 3);
    previewLayout->setColumnMinimumWidth(0, previewBounds->left() - 3);
    previewLayout->setColumnMinimumWidth(2, splashImage->width() - previewBounds->right());
    previewLayout->setRowMinimumHeight(2, splashImage->height() - previewBounds->bottom());
}

void MachineSplash::resizeEvent(QResizeEvent * event)
{
    doResize();
    QWidget::resizeEvent(event);
}

void MachineSplash::getPreviewRect()
{
    QRectF *fullsizeBounds = new QRectF( splashImage->renderer()->boundsOnElement("QtEmu_Preview_Screen"));
    float scaleFactor = 
        splashImage->width() / splashImage->renderer()->viewBoxF().width();
    previewBounds = new QRectF(
        fullsizeBounds->left()*scaleFactor,
        fullsizeBounds->top()*scaleFactor,
        fullsizeBounds->width()*scaleFactor,
        fullsizeBounds->height()*scaleFactor
        );
}


