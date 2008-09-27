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

#ifndef FLOATINGTOOLBAR_H
#define FLOATINGTOOLBAR_H

#include <QToolBar>

/**
 * @short A toolbar widget that slides in from a side.
 *
 * This is a shaped widget that slides in from a side of the 'anchor widget'
 * it's attached to. It can be dragged and docked on {left,top,right,bottom}
 * sides and contains actions.
 */
class FloatingToolBar : public QToolBar
{
    Q_OBJECT
public:
    FloatingToolBar(QWidget *parent, QWidget *anchorWidget);
    ~FloatingToolBar();

    Q_ENUMS(Side)
    enum Side { Left = 0, Top = 1, Right = 2, Bottom = 3 };

    void addAction(QAction *action);
    void setSide(Side side);

Q_SIGNALS:
    void orientationChanged(int side);

public Q_SLOTS:
    void setSticky(bool sticky);
    void showAndAnimate();
    void hideAndDestroy();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

private:
    class FloatingToolBarPrivate *d;

private Q_SLOTS:
    void animate();
    void hide();
};

#endif
