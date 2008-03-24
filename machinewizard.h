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

#include <QWizard>

class QComboBox;
class QLineEdit;
class QDoubleSpinBox;
class QCheckBox;

class MachineWizard : public QWizard
{
    Q_OBJECT

public:
    explicit MachineWizard(const QString &myMachinesPath, QWidget *parent = 0);

    static QString newMachine(const QString &myMachinesPathParent, QWidget *parent);

    void accept();

    QString myMachinesPath;
};

class ChooseSystemPage : public QWizardPage
{
    Q_OBJECT

public:
    ChooseSystemPage(MachineWizard *wizard);

    bool isComplete() const;

private:
    QComboBox *comboSystem;
};

class LocationPage : public QWizardPage
{
    Q_OBJECT

public:
    LocationPage(MachineWizard *wizard);

    void initializePage();
    bool isComplete() const;
    QLineEdit *pathLineEdit;
    QLineEdit *nameLineEdit;

private slots:
    void updatePath();
    void setNewPath();
};

class ImagePage : public QWizardPage
{
    Q_OBJECT

public:
    ImagePage(MachineWizard *wizard);

    void cleanupPage();
    bool isComplete() const;

private:
    QDoubleSpinBox *sizeSpinBox;
    QCheckBox *encryptionCheckBox;
    
private slots:
    //void enableEncryption(int choice);
};

#endif
