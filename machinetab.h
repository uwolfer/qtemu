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
    
public slots:
    void restart();

private:
    MachineProcess *machineProcess;
    QDomDocument domDocument;
    QCheckBox *snapshotCheckBox;
    QCheckBox *networkCheckBox;
    QCheckBox *soundCheckBox;
    QCheckBox *videoCheckBox;
    QCheckBox *videoResizeCheckBox;
    QRadioButton *soundALSARadioButton;
    QRadioButton *soundOSSRadioButton;
    QRadioButton *soundESDRadioButton;
    QButtonGroup *soundSystemGroup;
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
    QLineEdit *smbFolderEdit;
    QLineEdit *additionalOptionsEdit;
    QLineEdit *consoleCommand;
    QTextEdit *console;
    QSlider *memorySlider;
    QString xmlFileName;
    QTabWidget *parentTabWidget;
    QTextEdit *notesTextEdit;
    QPushButton *memoryButton;
    QFrame *memoryFrame;
    QPushButton *hddButton;
    QFrame *hddFrame;
    QPushButton *cdromButton;
    QPushButton *cdromReloadButton;
    QFrame *cdromFrame;
    QPushButton *floppyButton;
    QPushButton *floppyReloadButton;
    QFrame *floppyFrame;
    QPushButton *networkButton;
    QFrame *networkFrame;
    QCheckBox *userModeNetwork;
    QCheckBox *bridgedModeNetwork;
    QCheckBox *localBridgedModeNetwork;
    QPushButton *soundButton;
    QFrame *soundFrame;
    QPushButton *otherButton;
    QFrame *otherFrame;
    QString myMachinesPath;
    
    MachineView *machineView;
    QFrame *viewFrame;
    QFrame *consoleFrame;
    QFrame *settingsFrame;
    QGridLayout *viewLayout;
    QScrollArea *machineScroll;
    void cleanupView();

private slots:
    void start();
    void stop();
    void suspending();
    void suspended();
    void resuming();
    void resumed();
    void finished();
    void started();
    void booting();
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
    void setNewSmbFolderPath();
    void closeAllSections();
    bool read();
    bool write();
    void changeValue(const QString &name, const QString &value);
    void nameChanged(const QString &name);
    void closeMachine();
    void snapshot(const int state);
    void network(const int value);
    void unimplemented();
    void supressAudioErrors();
    void setSoundSystem(int id);
    void runCommand();
    void clearRestart();
    void enableScaling(int buttonState);
    void setupVnc(int enable);
    void updatePreview(const QString &hdPath);
    void takeScreenshot();
};

#endif
