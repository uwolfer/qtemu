/****************************************************************************
**
** Copyright (C) 2007-2008 Urs Wolfer <uwolfer @ kde.org>
** Parts of this file have been take from okular:
** Copyright (C) 2004-2005 Enrico Ros <eros.kde@email.it>
**
** This file is part of KDE.
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
** You should have received a copy of the GNU General Public License
** along with this program; see the file COPYING. If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#include "floatingtoolbar.h"

#ifndef QTONLY
    #include <KDebug>
#else
    #include <QDebug>
    #define kDebug(n) qDebug()
#endif

#include <QApplication>
#include <QBitmap>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

static const int actionIconSize = 22;
static const int toolBarRBMargin = 2;
static const double toolBarOpacity = 0.8;
static const int visiblePixelWhenAutoHidden = 6;
static const int autoHideTimeout = 500;
static const int  initialAutoHideTimeout = 2000;

/**
 * Denotes the verious states of the animation.
 */
enum AnimState {
    Hiding,
    Showing,
    Still
};

class FloatingToolBarPrivate
{
public:
    FloatingToolBarPrivate(FloatingToolBar *qq)
            : q(qq)
            , anchorSide(FloatingToolBar::Left)
            , offsetPlaceHolder(new QWidget(qq))
            , animState(Still)
            , toDelete(false)
            , visible(false)
            , sticky(false)
            , opacity(toolBarOpacity)
            , queuedShow(false) {
    }

    // rebuild contents and reposition then widget
    void buildToolBar();
    void reposition();
    // compute the visible and hidden positions along current side
    QPoint getInnerPoint() const;
    QPoint getOuterPoint() const;

    FloatingToolBar *q;

    QWidget *anchorWidget;
    FloatingToolBar::Side anchorSide;
    QWidget *offsetPlaceHolder;

    QTimer *animTimer;
    QTimer *autoHideTimer;
    QPoint currentPosition;
    QPoint endPosition;
    AnimState animState;
    bool toDelete;
    bool visible;
    bool sticky;
    qreal opacity;
    bool queuedShow;

    QPixmap backgroundPixmap;
};

FloatingToolBar::FloatingToolBar(QWidget *parent, QWidget *anchorWidget)
        : QToolBar(parent), d(new FloatingToolBarPrivate(this))
{
    ;
    addWidget(d->offsetPlaceHolder);

    setMouseTracking(true);
    setIconSize(QSize(actionIconSize, actionIconSize));
    d->anchorWidget = anchorWidget;

    d->animTimer = new QTimer(this);
    connect(d->animTimer, SIGNAL(timeout()), this, SLOT(animate()));

    d->autoHideTimer = new QTimer(this);
    connect(d->autoHideTimer, SIGNAL(timeout()), this, SLOT(hide()));

    // apply a filter to get notified when anchor changes geometry
    d->anchorWidget->installEventFilter(this);
}

FloatingToolBar::~FloatingToolBar()
{
    delete d;
}

void FloatingToolBar::addAction(QAction *action)
{
    QToolBar::addAction(action);

    // rebuild toolbar shape and contents only if the toolbar is already visible,
    // otherwise it will be done in showAndAnimate()
    if (isVisible())
        d->reposition();
}

void FloatingToolBar::setSide(Side side)
{
    d->anchorSide = side;

    if (isVisible())
        d->reposition();
}

void FloatingToolBar::setSticky(bool sticky)
{
    d->sticky = sticky;

    if (sticky)
        d->autoHideTimer->stop();
}

void FloatingToolBar::showAndAnimate()
{
    QDesktopWidget *desktop = QApplication::desktop();
    int currentScreen = desktop->screenNumber(d->anchorWidget);
    if ((d->anchorWidget->size() != desktop->screenGeometry(currentScreen).size())) {
        kDebug(5010) << "anchorWidget not fullscreen yet";
        d->queuedShow = true;
        return;
    }
    if (d->animState == Showing)
        return;

    d->animState = Showing;

    show();

    // force update for case when toolbar has not been built yet
    d->reposition();

    // start scrolling in
    d->animTimer->start(20);

    // This permits to show the toolbar for a while when going full screen.
    if (!d->sticky) 
        d->autoHideTimer->start(initialAutoHideTimeout);
}

void FloatingToolBar::hideAndDestroy()
{
    if (d->animState == Hiding)
        return;

    // set parameters for sliding out
    d->animState = Hiding;
    d->toDelete = true;
    d->endPosition = d->getOuterPoint();

    // start scrolling out
    d->animTimer->start(20);
}

void FloatingToolBar::hide()
{
    if (underMouse()) 
        return;

    if (d->visible) {
        QPoint diff;
        switch (d->anchorSide) {
        case Left:
            diff = QPoint(visiblePixelWhenAutoHidden, 0);
            break;
        case Right:
            diff = QPoint(-visiblePixelWhenAutoHidden, 0);
            break;
        case Top:
            diff = QPoint(0, visiblePixelWhenAutoHidden);
            break;
        case Bottom:
            diff = QPoint(0, -visiblePixelWhenAutoHidden);
            break;
        }
        d->animState = Hiding;
        d->endPosition = d->getOuterPoint() + diff;

        // start scrolling out
        d->animTimer->start(20);
    }
}

bool FloatingToolBar::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == d->anchorWidget && e->type() == QEvent::Resize) {
        if (d->queuedShow) { // if the toolbar is not visible yet, try to show it if the anchor widget is in fullscreen already
            d->queuedShow = false;
            showAndAnimate();
            return true;
        }
        
        // if anchorWidget changed geometry reposition toolbar
        d->animTimer->stop();
        if ((d->animState == Hiding || !d->visible) && d->toDelete)
            deleteLater();
        else
            d->reposition();
    }

    return QToolBar::eventFilter(obj, e);
}

void FloatingToolBar::paintEvent(QPaintEvent *e)
{
    QToolBar::paintEvent(e);

    // paint the internal pixmap over the widget
    QPainter p(this);
    p.setOpacity(d->opacity);
    p.drawImage(e->rect().topLeft(), d->backgroundPixmap.toImage(), e->rect());
}

void FloatingToolBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        setCursor(Qt::SizeAllCursor);

    QToolBar::mousePressEvent(e);
}

void FloatingToolBar::mouseMoveEvent(QMouseEvent *e)
{
    // show the toolbar again when it is auto-hidden
    if (!d->visible) {
        showAndAnimate();
        return;
    }

    if ((QApplication::mouseButtons() & Qt::LeftButton) != Qt::LeftButton)
        return;

    // compute the nearest side to attach the widget to
    const QPoint parentPos = mapToParent(e->pos());
    const float nX = (float)parentPos.x() / (float)d->anchorWidget->width();
    const float nY = (float)parentPos.y() / (float)d->anchorWidget->height();
    if (nX > 0.3 && nX < 0.7 && nY > 0.3 && nY < 0.7)
        return;
    bool LT = nX < (1.0 - nY);
    bool LB = nX < (nY);
    Side side = LT ? (LB ? Left : Top) : (LB ? Bottom : Right);

    // check if side changed
    if (side == d->anchorSide)
        return;

    d->anchorSide = side;
    d->reposition();
    emit orientationChanged((int)side);

    QToolBar::mouseMoveEvent(e);
}

void FloatingToolBar::enterEvent(QEvent *e)
{
    // Stop the autohide timer while the mouse is inside
    d->autoHideTimer->stop();

    if (!d->visible) 
        showAndAnimate();
    QToolBar::enterEvent(e);
}

void FloatingToolBar::leaveEvent(QEvent *e)
{
    if (!d->sticky) 
        d->autoHideTimer->start(autoHideTimeout);
    QToolBar::leaveEvent(e);
}

void FloatingToolBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        setCursor(Qt::ArrowCursor);

    QToolBar::mouseReleaseEvent(e);
}

void FloatingToolBar::wheelEvent(QWheelEvent *e)
{
    e->accept();

    const qreal diff = e->delta() / 100.0 / 15.0;
//     kDebug(5010) << diff;
    if (((d->opacity <= 1) && (diff > 0)) || ((d->opacity >= 0) && (diff < 0)))
        d->opacity += diff;

    update();

    QToolBar::wheelEvent(e);
}

void FloatingToolBarPrivate::buildToolBar()
{
    const bool prevUpdates = q->updatesEnabled();
    q->setUpdatesEnabled(false);

    // 1. init numbers we are going to use
    const bool topLeft = anchorSide == FloatingToolBar::Left || anchorSide == FloatingToolBar::Top;
    const bool vertical = anchorSide == FloatingToolBar::Left || anchorSide == FloatingToolBar::Right;

    if (anchorSide == FloatingToolBar::Left || anchorSide == FloatingToolBar::Right) {
        offsetPlaceHolder->setFixedSize(1, 7);
        q->setOrientation(Qt::Vertical);
    } else {
        offsetPlaceHolder->setFixedSize(7, 1);
        q->setOrientation(Qt::Horizontal);
    }

    // 2. compute widget size
    const int myWidth = q->sizeHint().width() - 1;
    const int myHeight = q->sizeHint().height() - 1;

    // 3. resize pixmap, mask and widget
    QBitmap mask(myWidth + 1, myHeight + 1);
    backgroundPixmap = QPixmap(myWidth + 1, myHeight + 1);
    backgroundPixmap.fill(Qt::transparent);

    q->resize(myWidth + 1, myHeight + 1);

    // 4. create and set transparency mask
    QPainter maskPainter(&mask);
    mask.fill(Qt::white);
    maskPainter.setBrush(Qt::black);
    if (vertical)
        maskPainter.drawRoundRect(topLeft ? -10 : 0, 0, myWidth + 10, myHeight, 2000 / (myWidth + 10), 2000 / myHeight);
    else
        maskPainter.drawRoundRect(0, topLeft ? -10 : 0, myWidth, myHeight + 10, 2000 / myWidth, 2000 / (myHeight + 10));
    maskPainter.end();
    q->setMask(mask);

    // 5. draw background
    QPainter bufferPainter(&backgroundPixmap);
    bufferPainter.translate(0.5, 0.5);
    QPalette pal = q->palette();
    // 5.1. draw horizontal/vertical gradient
    QLinearGradient grad;
    switch (anchorSide) {
    case FloatingToolBar::Left:
        grad = QLinearGradient(0, 1, myWidth + 1, 1);
        break;
    case FloatingToolBar::Right:
        grad = QLinearGradient(myWidth + 1, 1, 0, 1);
        break;
    case FloatingToolBar::Top:
        grad = QLinearGradient(1, 0, 1, myHeight + 1);
        break;
    case FloatingToolBar::Bottom:
        grad = QLinearGradient(1, myHeight + 1, 0, 1);
        break;
    }
    grad.setColorAt(0, pal.color(QPalette::Active, QPalette::Button));
    grad.setColorAt(1, pal.color(QPalette::Active, QPalette::Light));
    bufferPainter.setBrush(QBrush(grad));
    // 5.2. draw rounded border
    bufferPainter.setPen( pal.color(QPalette::Active, QPalette::Dark).lighter(40));
    bufferPainter.setRenderHints(QPainter::Antialiasing);
    if (vertical)
        bufferPainter.drawRoundRect(topLeft ? -10 : 0, 0, myWidth + 10, myHeight, 2000 / (myWidth + 10), 2000 / myHeight);
    else
        bufferPainter.drawRoundRect(0, topLeft ? -10 : 0, myWidth, myHeight + 10, 2000 / myWidth, 2000 / (myHeight + 10));
    // 5.3. draw handle
    bufferPainter.translate(-0.5, -0.5);
    bufferPainter.setPen(pal.color(QPalette::Active, QPalette::Mid));
    if (vertical) {
        int dx = anchorSide == FloatingToolBar::Left ? 2 : 4;
        bufferPainter.drawLine(dx, 6, dx + myWidth - 8, 6);
        bufferPainter.drawLine(dx, 9, dx + myWidth - 8, 9);
        bufferPainter.setPen(pal.color(QPalette::Active, QPalette::Light));
        bufferPainter.drawLine(dx + 1, 7, dx + myWidth - 7, 7);
        bufferPainter.drawLine(dx + 1, 10, dx + myWidth - 7, 10);
    } else {
        int dy = anchorSide == FloatingToolBar::Top ? 2 : 4;
        bufferPainter.drawLine(6, dy, 6, dy + myHeight - 8);
        bufferPainter.drawLine(9, dy, 9, dy + myHeight - 8);
        bufferPainter.setPen(pal.color(QPalette::Active, QPalette::Light));
        bufferPainter.drawLine(7, dy + 1, 7, dy + myHeight - 7);
        bufferPainter.drawLine(10, dy + 1, 10, dy + myHeight - 7);
    }

    q->setUpdatesEnabled(prevUpdates);
}

void FloatingToolBarPrivate::reposition()
{
    // note: hiding widget here will gives better gfx, but ends drag operation
    // rebuild widget and move it to its final place
    buildToolBar();
    if (!visible) {
        currentPosition = getOuterPoint();
        endPosition = getInnerPoint();
    } else {
        currentPosition = getInnerPoint();
        endPosition = getOuterPoint();
    }
    q->move(currentPosition);
}

QPoint FloatingToolBarPrivate::getInnerPoint() const
{
    // returns the final position of the widget
    if (anchorSide == FloatingToolBar::Left)
        return QPoint(0, (anchorWidget->height() - q->height()) / 2);
    if (anchorSide == FloatingToolBar::Top)
        return QPoint((anchorWidget->width() - q->width()) / 2, 0);
    if (anchorSide == FloatingToolBar::Right)
        return QPoint(anchorWidget->width() - q->width() + toolBarRBMargin, (anchorWidget->height() - q->height()) / 2);
    return QPoint((anchorWidget->width() - q->width()) / 2, anchorWidget->height() - q->height() + toolBarRBMargin);
}

QPoint FloatingToolBarPrivate::getOuterPoint() const
{
    // returns the point from which the transition starts
    if (anchorSide == FloatingToolBar::Left)
        return QPoint(-q->width(), (anchorWidget->height() - q->height()) / 2);
    if (anchorSide == FloatingToolBar::Top)
        return QPoint((anchorWidget->width() - q->width()) / 2, -q->height());
    if (anchorSide == FloatingToolBar::Right)
        return QPoint(anchorWidget->width() + toolBarRBMargin, (anchorWidget->height() - q->height()) / 2);
    return QPoint((anchorWidget->width() - q->width()) / 2, anchorWidget->height() + toolBarRBMargin);
}

void FloatingToolBar::animate()
{
    // move currentPosition towards endPosition
    int dX = d->endPosition.x() - d->currentPosition.x();
    int dY = d->endPosition.y() - d->currentPosition.y();
    dX = dX / 6 + qMax(-1, qMin(1, dX));
    dY = dY / 6 + qMax(-1, qMin(1, dY));
    d->currentPosition.setX(d->currentPosition.x() + dX);
    d->currentPosition.setY(d->currentPosition.y() + dY);

    move(d->currentPosition);

    // handle arrival to the end
    if (d->currentPosition == d->endPosition) {
        d->animTimer->stop();
        switch (d->animState) {
        case Hiding:
            d->visible = false;
            d->animState = Still;
            if (d->toDelete)
                deleteLater();
            break;
        case Showing:
            d->visible = true;
            d->animState = Still;
            break;
        default:
            kDebug(5010) << "Illegal state";
        }
    }
}
