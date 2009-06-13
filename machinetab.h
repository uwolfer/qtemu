/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
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

#ifndef MACHINETAB_H
#define MACHINETAB_H

#include <QWidget>
#include <QDomDocument>
#include <QRadioButton>
#include <QButtonGroup>

#include "machineconfigobject.h"
#include "machineconfig.h"

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
class MachineView;
class QVBoxLayout;
class QGridLayout;
class QScrollArea;
class QToolButton;
class SettingsTab;
class ControlPanel;
class GuestToolsListener;

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
    MachineConfigObject *machineConfigObject;
    SettingsTab *settingsTab;
    MachineProcess *machineProcess;
    MachineView *machineView;

public slots:
    void restart();
	

private:

    void makeConnections();

    MachineConfig *machineConfig;

    QCheckBox *snapshotCheckBox;
    
    QLineEdit *machineNameEdit;
    
    QLineEdit *consoleCommand;
    QTextEdit *console;
    
    QString xmlFileName;
    QTabWidget *parentTabWidget;
    QTextEdit *notesTextEdit;
    
    QPushButton *cdromReloadButton;

    QPushButton *floppyReloadButton;

    QString myMachinesPath;

    QStringList shownErrors;

    QFrame *consoleFrame;
    QGridLayout *viewLayout;
    QScrollArea *machineScroll;
    ControlPanel *controlPanel;

    void cleanupView();
   
    GuestToolsListener *guestToolsListener;

private slots:
    void start();
    void forceStop();
    void suspending();
    void suspended();
    void resuming();
    void resumed();
    void finished();
    void started();
    void booting();
    void error(const QString& errorMsg);

    bool read();
    void nameChanged(const QString &name);
    void closeMachine();
    void snapshot(const int state);
    void runCommand();
    void clearRestart();
    void takeScreenshot();

};

#endif
