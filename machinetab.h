/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
**                         Ben Klopfenstein <benklop @ gmail.com>
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

#ifndef MACHINETAB_H
#define MACHINETAB_H

#include <QWidget>
#include <QDomDocument>

class QPushButton;
class QLineEdit;
class QCheckBox;
class QSlider;
class QSpinBox;
class QTabWidget;
class QTextEdit;
class QFrame;
class QPushButton;
class QMenu;
class MachineProcess;

class MachineTab : public QWidget
{
    Q_OBJECT
public:
    MachineTab(QTabWidget *parent, const QString &fileName, const QString &myMachinesPathParent);
    QString machineName();
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *suspendButton;
    QPushButton *resumeButton;
    QPushButton *pauseButton;


private:
    MachineProcess *machineProcess;
    QDomDocument domDocument;
    QCheckBox *snapshotCheckBox;
    QCheckBox *networkCheckBox;
    QCheckBox *soundCheckBox;
    QCheckBox *soundSystemCheckBox;
    QCheckBox *cdBootCheckBox;
    QCheckBox *floppyBootCheckBox;
    QCheckBox *mouseCheckBox;
    QCheckBox *timeCheckBox;
    QCheckBox *virtualizationCheckBox;
    QCheckBox *additionalOptionsCheckBox;
    QSpinBox *cpuSpinBox;
    QLineEdit *machineNameEdit;
    QLineEdit *hddPathLineEdit;
    QPushButton *hddUpgradeButton;
    QLineEdit *cdromLineEdit;
    QLineEdit *floppyLineEdit;
    QLineEdit *networkCustomOptionsEdit;
    QLineEdit *additionalOptionsEdit;
    QSlider *memorySlider;
    QString xmlFileName;
    QTabWidget *parentTabWidget;
    QTextEdit *notesTextEdit;
    QPushButton *memoryButton;
    QFrame *memoryFrame;
    QPushButton *hddButton;
    QFrame *hddFrame;
    QPushButton *cdromButton;
    QFrame *cdromFrame;
    QPushButton *floppyButton;
    QFrame *floppyFrame;
    QPushButton *networkButton;
    QFrame *networkFrame;
    QPushButton *soundButton;
    QFrame *soundFrame;
    QPushButton *otherButton;
    QFrame *otherFrame;

    QString myMachinesPath;

private slots:
    void start();
    void stop();
    void suspending();
    void suspended();
    void resuming();
    void resumed();
    void finished();
    void started();
    void error(const QString& errorMsg);
    void setNewHddPath();
    void testHDDImage(const QString &path);
    void upgradeImage();
    void upgradeImageStarted();
    void upgradeImageFinished(const int &exitCode);
    void setNewCdRomPath();
    void setNewCdImagePath();
    void setNewFloppyDiskPath();
    void setNewFloppyDiskImagePath();
    void closeAllSections();
    bool read();
    bool write();
    void changeValue(const QString &name, const QString &value);
    void nameChanged(const QString &name);
    void closeMachine();
};

#endif
