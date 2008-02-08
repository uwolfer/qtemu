/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
**
** Some parts of this file have been taken from
** examples/dialogs/complexwizard of Qt 4.1 which is
** Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
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

#ifndef WIZARD_H
#define WIZARD_H

#include <QDialog>
#include <QList>

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class QFrame;
class QLabel;
class WizardPage;

class Wizard : public QDialog
{
    Q_OBJECT

public:
    Wizard(QWidget *parent = 0);

    QList<WizardPage *> historyPages() const { return history; }

public slots:
    void setTitle(const QString &title);

signals:
    void finished();

protected:
    void setFirstPage(WizardPage *page);

private slots:
    void backButtonClicked();
    void nextButtonClicked();
    void completeStateChanged();

private:
    void switchPage(WizardPage *oldPage);

    QList<WizardPage *> history;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    QFrame *headerFrame;
    QLabel *headerLabel;
    QLabel *headerIcon;
};

class WizardPage : public QWidget
{
    Q_OBJECT

public:
    WizardPage(QWidget *parent = 0);

    virtual void updateTitle();
    virtual void resetPage();
    virtual WizardPage *nextPage();
    virtual bool isLastPage();
    virtual bool isComplete();

signals:
    void completeStateChanged();

private slots:
    virtual void privateSlot();
};

#endif
