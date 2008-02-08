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

#include "wizard.h"

#include <QPushButton>
#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QSettings>

Wizard::Wizard(QWidget *parent)
    : QDialog(parent)
{
    cancelButton = new QPushButton(tr("Cancel"));
    backButton = new QPushButton(tr("< &Back"));
    nextButton = new QPushButton(tr("Next >"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(nextButton);

    headerFrame = new QFrame;
    headerFrame->setFrameShape(QFrame::StyledPanel);
    headerFrame->setFrameShadow(QFrame::Plain);
    headerFrame->setAutoFillBackground(true);
    headerFrame->setBackgroundRole(QPalette::Base);

    QHBoxLayout *headerLayout = new QHBoxLayout;

    headerLabel = new QLabel;
#if QT_VERSION >= 0x040200
    headerLabel->setStyleSheet("QLabel { font-weight: bold; }");
#endif
    headerLayout->addWidget(headerLabel);

    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();

    headerIcon = new QLabel;
    headerIcon->setPixmap(QPixmap(":/images/" + iconTheme + "/qtemu.png"));
    headerLayout->addWidget(headerIcon);

    headerLayout->setStretchFactor(headerLabel, 1);
    headerFrame->setLayout(headerLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(headerFrame);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

void Wizard::setTitle(const QString &title)
{
    headerLabel->setText(title);
}

void Wizard::setFirstPage(WizardPage *page)
{
    page->resetPage();
    history.append(page);
    switchPage(0);
}

void Wizard::backButtonClicked()
{
    WizardPage *oldPage = history.takeLast();
    oldPage->resetPage();
    switchPage(oldPage);
}

void Wizard::nextButtonClicked()
{
    WizardPage *oldPage = history.last();
    WizardPage *newPage = oldPage->nextPage();
    newPage->resetPage();
    history.append(newPage);
    switchPage(oldPage);
}

void Wizard::completeStateChanged()
{
    nextButton->setDefault(true);
    WizardPage *currentPage = history.last();
    nextButton->setEnabled(currentPage->isComplete());
    if (currentPage->isLastPage())
        nextButton->setText(tr("&Finish"));
    else
        nextButton->setText(tr("Next >"));
}

void Wizard::switchPage(WizardPage *oldPage)
{
    if (oldPage)
    {
        oldPage->hide();
        mainLayout->removeWidget(oldPage);
        disconnect(oldPage, SIGNAL(completeStateChanged()),
                   this, SLOT(completeStateChanged()));
    }

    WizardPage *newPage = history.last();
    mainLayout->insertWidget(1, newPage);
    newPage->show();
    newPage->setFocus();
    newPage->updateTitle();
    connect(newPage, SIGNAL(completeStateChanged()),
            this, SLOT(completeStateChanged()));

    backButton->setEnabled(history.size() != 1);
    if (newPage->isLastPage())
    {
        disconnect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
        connect(nextButton, SIGNAL(clicked()), this, SIGNAL(finished()));
        connect(nextButton, SIGNAL(clicked()), this, SLOT(accept()));
    }
    else
    {
        disconnect(nextButton, SIGNAL(clicked()), this, SIGNAL(finished()));
        disconnect(nextButton, SIGNAL(clicked()), this, SLOT(accept()));
        disconnect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
        connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
    }
    completeStateChanged();
}

WizardPage::WizardPage(QWidget *parent)
    : QWidget(parent)
{
    hide();
}

void WizardPage::updateTitle()
{
}

void WizardPage::resetPage()
{
}

WizardPage *WizardPage::nextPage()
{
    return 0;
}

bool WizardPage::isLastPage()
{
    return false;
}

bool WizardPage::isComplete()
{
    return true;
}

void WizardPage::privateSlot()
{
}
