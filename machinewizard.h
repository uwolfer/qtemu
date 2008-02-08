/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
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

#ifndef MACHINEWIZARD_H
#define MACHINEWIZARD_H

#include "wizard.h"

class ChooseSystemPage;
class LocationPage;
class ImagePage;
class QComboBox;
class QLineEdit;
class QDoubleSpinBox;
class QProcess;
class QDir;

class MachineWizard : public Wizard
{
    Q_OBJECT
public:
    explicit MachineWizard(const QString &myMachinesPath, QWidget *parent = 0);

    static QString newMachine(const QString &myMachinesPathParent, QWidget *parent);

    QString osName;
    QString osNameUser;
    QString osPathUser;
    QString myMachinesPath;

private:
    ChooseSystemPage *chooseSystemPage;
    LocationPage *locationPage;
    ImagePage *imagePage;

    friend class ChooseSystemPage;
    friend class LocationPage;
    friend class ImagePage;
};

class MachineWizardPage : public WizardPage
{
    Q_OBJECT
public:
    MachineWizardPage(MachineWizard *wizard)
        : WizardPage(wizard), wizard(wizard) {}

protected:
    MachineWizard *wizard;
};

class ChooseSystemPage : public MachineWizardPage
{
    Q_OBJECT
public:
    ChooseSystemPage(MachineWizard *wizard);

    void updateTitle();
    void resetPage();
    bool isComplete();
    WizardPage *nextPage();

private:
    QComboBox *comboSystem;
};

class LocationPage : public MachineWizardPage
{
    Q_OBJECT
public:
    LocationPage(MachineWizard *wizard);

    void updateTitle();
    void resetPage();
    bool isComplete();
    WizardPage *nextPage();
    QLineEdit *pathLineEdit;
    QLineEdit *nameLineEdit;

private:
    QLabel *nameLabel;
    QLabel *pathLabel;

private slots:
    void privateSlot();
    void setNewPath();
};

class ImagePage : public MachineWizardPage
{
    Q_OBJECT
public:
    ImagePage(MachineWizard *wizard);

    void updateTitle();
    void resetPage();
    bool isComplete();
    bool isLastPage() { return true; }

private:
    QLabel *sizeLabel;
    QLabel *sizeGbLabel;
    QDoubleSpinBox *sizeSpinBox;
    QProcess *imageCreateProcess;
    QDir *dir;

private slots:
    void privateSlot();
};

#endif
