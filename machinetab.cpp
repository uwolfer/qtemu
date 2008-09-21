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

#include "machinetab.h"

#include "machineprocess.h"

#include "config.h"

#include "machineview.h"

#include "settingstab.h"

#include "harddiskmanager.h"

#include "controlpanel.h"

#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QFile>
#include <QCheckBox>
#include <QSlider>
#include <QTextEdit>
#include <QSpinBox>
#include <QFileDialog>
#include <QTextStream>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QFileInfo>
#include <QMenu>
#include <QButtonGroup>
#include <QTimer>
#include <QResizeEvent>
#include <QScrollArea>
#include <QUrl>
#include <QToolButton>
#include <QAction>

MachineTab::MachineTab(QTabWidget *parent, const QString &fileName, const QString &myMachinesPathParent)
    : QWidget(parent)
{
    parentTabWidget = parent;
    xmlFileName = fileName;
    myMachinesPath = myMachinesPathParent;

    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();

    machineConfig = new MachineConfig(this, fileName);
    machineConfigObject = new MachineConfigObject(this, machineConfig);

    machineProcess = new MachineProcess(this);
    machineConfigObject->registerObject(machineProcess);
    machineConfigObject->registerObject(machineProcess->getHdManager());

    machineView = new MachineView(this);

    machineConfigObject->setOption("vncPort", 1000 + parentTabWidget->currentIndex() + 1);

    settingsTab = new SettingsTab(machineConfigObject, this);


    connect(machineProcess, SIGNAL(finished(int)), this, SLOT(finished()));
    connect(machineProcess, SIGNAL(started()), this, SLOT(started()));
    connect(machineProcess, SIGNAL(suspending(QString)), this, SLOT(suspending()));
    connect(machineProcess, SIGNAL(suspended(QString)), this, SLOT(suspended()));
    connect(machineProcess, SIGNAL(resuming(QString)), this, SLOT(resuming()));
    connect(machineProcess, SIGNAL(resumed(QString)), this, SLOT(resumed()));
    connect(machineProcess, SIGNAL(error(QString)), this, SLOT(error(QString)));
    connect(machineProcess, SIGNAL(booting()), this, SLOT(booting()));

    machineNameEdit = new QLineEdit(this);

#ifndef Q_OS_WIN32
    const QString flatStyle = QString("TYPE { border: 2px solid transparent;"
                                      "background-color: transparent; }"
                                      "TYPE:hover { background-color: white;"
                                      "border: 2px inset %1; border-radius: 3px;}"
                                      "TYPE:focus { background-color: white;"
                                      "border: 2px inset %2; border-radius: 3px;}")
                                      .arg(machineNameEdit->palette().color(QPalette::Mid).name())
                                      .arg(machineNameEdit->palette().color(QPalette::Highlight).name());
#elif defined(Q_OS_WIN32)
    const QString flatStyle = QString("TYPE { border: 1px solid transparent;"
                                      "background-color: transparent; }"
                                      "TYPE:hover, TYPE:focus { background-color: white;"
                                      "border: 1px solid %1;}")
                                      .arg(machineNameEdit->palette().color(QPalette::Highlight).name());
#endif
#if QT_VERSION >= 0x040200
    machineNameEdit->setStyleSheet(QString(flatStyle).replace("TYPE", "QLineEdit")
                                   +"QLineEdit { font: bold 16px; }");
#endif

    QPushButton *closeButton = new QPushButton(QIcon(":/images/" + iconTheme + "/close.png"), QString(), this);
    closeButton->setFlat(true);
    closeButton->setToolTip(tr("Close this machine"));
    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(closeMachine()));

    QHBoxLayout *closeButtonLayout = new QHBoxLayout;
    closeButtonLayout->addWidget(machineNameEdit);
    closeButtonLayout->addWidget(closeButton);

    startButton = new QPushButton(QIcon(":/images/" + iconTheme + "/start.png"), tr("&Start"), this);
    startButton->setWhatsThis(tr("Start this virtual machine"));
    startButton->setIconSize(QSize(22, 22));
    connect(startButton, SIGNAL(clicked(bool)), this, SLOT(start()));

    QMenu *stopMenu = new QMenu();
    QAction *stopAction = stopMenu->addAction(QIcon(":/images/" + iconTheme + "/stop.png"), tr("&Shutdown"));
    QAction *forceAction = stopMenu->addAction(tr("&Force Poweroff"));
    stopAction->setWhatsThis(tr("Tell this virtual machine to shut down"));
    forceAction->setWhatsThis(tr("Force this virtual machine to stop immediately"));
    stopButton = new QToolButton(this);
    stopButton->setMenu(stopMenu);
    stopButton->setDefaultAction(stopAction);

    stopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    stopButton->setIconSize(QSize(22, 22));
    stopButton->setSizePolicy(startButton->sizePolicy());
    stopButton->setText(tr("&Stop"));


    stopButton->setEnabled(false);
    connect(stopAction, SIGNAL(triggered(bool)), machineProcess, SLOT(stop()));
    connect(forceAction, SIGNAL(triggered(bool)), this, SLOT(forceStop()));

    QHBoxLayout *powerButtonsLayout = new QHBoxLayout;
    powerButtonsLayout->addWidget(startButton);
    powerButtonsLayout->addWidget(stopButton);

    suspendButton = new QPushButton(QIcon(":/images/" + iconTheme + "/suspend.png"), tr("&Suspend"), this);
    suspendButton->setWhatsThis(tr("Suspend this virtual machine"));
    suspendButton->setIconSize(QSize(22, 22));
    connect(suspendButton, SIGNAL(clicked(bool)), machineProcess, SLOT(suspend()));
    suspendButton->setHidden(true);
    suspendButton->setEnabled(false);

    resumeButton = new QPushButton(QIcon(":/images/" + iconTheme + "/resume.png"), tr("&Resume"), this);
    resumeButton->setWhatsThis(tr("Resume this virtual machine"));
    resumeButton->setIconSize(QSize(22, 22));
    connect(resumeButton, SIGNAL(clicked(bool)), machineProcess, SLOT(resume()));
    resumeButton->setEnabled(false);

    pauseButton = new QPushButton(QIcon(":/images/" + iconTheme + "/pause.png"), tr("&Pause"), this);
    pauseButton->setWhatsThis(tr("Pause/Unpause this virtual machine"));
    pauseButton->setCheckable(true);
    pauseButton->setIconSize(QSize(22, 22));
    pauseButton->setEnabled(false);
    connect(pauseButton, SIGNAL(clicked(bool)), machineProcess, SLOT(togglePause()));

    QHBoxLayout *controlButtonsLayout = new QHBoxLayout;
    controlButtonsLayout->addWidget(suspendButton);
    controlButtonsLayout->addWidget(resumeButton);
    controlButtonsLayout->addWidget(pauseButton);
    
    snapshotCheckBox = new QCheckBox(tr("Snapshot mode"), this);
    connect(snapshotCheckBox, SIGNAL(stateChanged(int)), this, SLOT(snapshot(int)));
    
    QToolButton *screenshotButton = new QToolButton(this);
    screenshotButton->setAutoRaise(false);
    screenshotButton->setIcon(QIcon(":/images/" + iconTheme + "/camera.png"));
    screenshotButton->setIconSize(QSize(22, 22));
    screenshotButton->setToolTip(tr("Set preview screenshot"));
    QHBoxLayout *snapshotLayout = new QHBoxLayout;
    snapshotLayout->addWidget(snapshotCheckBox);
    snapshotLayout->addWidget(screenshotButton);
    
    connect(screenshotButton, SIGNAL(clicked()), this, SLOT(takeScreenshot()));
    
    QLabel *notesLabel = new QLabel(tr("<strong>Notes</strong>"), this);

    notesTextEdit = new QTextEdit(this);
    notesTextEdit->setFixedHeight(60);
#if QT_VERSION >= 0x040200
    notesTextEdit->setStyleSheet(QString(flatStyle).replace("TYPE", "QTextEdit"));
#endif
    QLabel *controlLabel = new QLabel(tr("<strong>Control Panel</strong>"), this);

    controlPanel = new ControlPanel(this);


    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->addLayout(closeButtonLayout);
    buttonsLayout->addLayout(powerButtonsLayout);
    buttonsLayout->addLayout(controlButtonsLayout);
    buttonsLayout->addLayout(snapshotLayout);
    buttonsLayout->addWidget(notesLabel);
    buttonsLayout->addWidget(notesTextEdit);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(controlLabel);
    buttonsLayout->addWidget(controlPanel);


    //set up the layout for the tab panel
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addLayout(buttonsLayout, 0, 0);
    
    QTabWidget *viewTabs = new QTabWidget(this);
    mainLayout->addWidget(viewTabs, 0, 1);   
    mainLayout->setColumnStretch(1, 10);

    viewTabs->addTab(machineView, tr("Display"));


    machineConfigObject->registerObject(machineView);

    viewTabs->addTab(settingsTab, tr("Settings"));

//console area
    consoleFrame = new QFrame(this);
    viewTabs->addTab(consoleFrame, tr("Console"));
    console = new QTextEdit(this);
    console->setReadOnly(true);
    connect(machineProcess, SIGNAL(cleanConsole(QString)), console, SLOT(append(QString)));
    consoleCommand = new QLineEdit(this);
    //console->setFocusProxy(consoleCommand);
    QPushButton *consoleCommandButton = new QPushButton(tr("Enter Command"), this);
    connect(consoleCommand, SIGNAL(returnPressed()),
            consoleCommandButton, SLOT(click()));
    connect(consoleCommandButton, SIGNAL(clicked()), this, SLOT(runCommand()));
    QVBoxLayout *consoleLayout = new QVBoxLayout();
    QHBoxLayout *consoleCommandLayout = new QHBoxLayout();
    consoleLayout->addWidget(console);
    consoleCommandLayout->addWidget(consoleCommand);
    consoleCommandLayout->addWidget(consoleCommandButton);
    consoleLayout->addLayout(consoleCommandLayout);
    consoleFrame->setLayout(consoleLayout); 
    setLayout(mainLayout);

//end console area

    read();
    //read first the name, otherwise the name of the main tab changes
    
    makeConnections();
    machineProcess->getHdManager()->testImage();
}


bool MachineTab::read()
{
    //init and register values

    machineConfigObject->registerObject(machineNameEdit, "name");
    machineConfigObject->registerObject(snapshotCheckBox, "snapshot", QVariant(false));
#ifdef DEVELOPER
    machineConfigObject->setOption("snapshot", true);
#endif
    machineConfigObject->registerObject(notesTextEdit, "notes");
    return true;
}


void MachineTab::nameChanged(const QString &name)
{
    parentTabWidget->setTabText(parentTabWidget->currentIndex(), name);
}

QString MachineTab::machineName()
{
    return machineNameEdit->text();
}

void MachineTab::closeMachine()
{
    if (QMessageBox::question(this, tr("Close confirmation"),
                              tr("Are you sure you want to close this machine?<br />"
                                 "You can open it again with the corresponding .qte file in your \"MyMachines\" folder."),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No)
      == QMessageBox::Yes)
        parentTabWidget->removeTab(parentTabWidget->currentIndex());
}

void MachineTab::start()
{
    console->clear();
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    machineProcess->start();

}

void MachineTab::suspending()
{
    //startButton->setEnabled(false);
    //stopButton->setEnabled(false);
    //suspendButton->setEnabled(false);
    //pauseButton->setEnabled(false);
    setEnabled(false);
    //TODO: start a progress bar
}

void MachineTab::suspended()
    {
    machineProcess->forceStop();
    //resumeButton->setHidden(false);
    machineProcess->getHdManager()->testImage();
    suspendButton->setHidden(true);
    setEnabled(true);
    //stop the progress bar
    }

void MachineTab::resuming()
{
    //startButton->setEnabled(false);
    //stopButton->setEnabled(false);
    setEnabled(false);
}

void MachineTab::resumed()
{
    setEnabled(true);
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    suspendButton->setHidden(false);
    resumeButton->setHidden(true);

    //this is kinda sucky, i think it's a qemu bug.
    QMessageBox::information(this, tr("Resume"),
                             tr("Your machine is being resumed. USB devices will not function properly on Windows. You must reload<br />the USB driver to use your usb devices including the seamless mouse.<br />In addition the advanced VGA adapter will not refresh initially on any OS."));
}

void MachineTab::forceStop()
{
    QMessageBox msgBox;
    msgBox.setText(tr("This will force the current machine to power down. Are you sure?<br />"
                             "You should only do this if the machine is unresponsive or does not support ACPI. Doing this may cause damage to the disk image."));
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QPushButton *forceShutdownButton = msgBox.addButton(tr("Force Power Off"), QMessageBox::DestructiveRole);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    if (msgBox.clickedButton()==forceShutdownButton)
        machineProcess->forceStop();
}

void MachineTab::finished()
{
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    pauseButton->setEnabled(false);
    resumeButton->setHidden(false);
    suspendButton->setHidden(true);
    snapshotCheckBox->setText(tr("Snapshot mode"));
    cleanupView();
    machineProcess->getHdManager()->testImage();
    shownErrors.clear();
}

void MachineTab::started()
{

}

void MachineTab::error(const QString & errorMsg)
{
    if (errorMsg.contains("(VMDK)"))
    {
        //for some reason qemu throws an error message when using a VMDK image. oh well... it isn't a REAL error, so ignore it.
        return;
    }
    else if (errorMsg.contains("bind()"))
    {
        if( QMessageBox::critical(this, tr("QtEmu machine already running!"), tr("There is already a virtual machine running on the specified<br />"
                                                                             "VNC port or file. This may mean a previous QtEmu session crashed; <br />"
                                                                             "if this is the case you can try to connect to the virtual machine <br />"
                                                                             "to rescue your data and shut it down safely.<br /><br />"
                                                                             "Try to connect to this machine?"),QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            QTimer::singleShot(500, machineView, SLOT(initView()));
        }
        return;
    }
    else if ((!shownErrors.contains("audio"))&&(errorMsg.contains("audio:")))
    {
        shownErrors << "audio";
        QMessageBox::critical(this, tr("QtEmu Sound Error"), tr("QtEmu is having trouble accessing your sound system. Make sure<br />"
                                                                "you have your host sound system selected correctly in the Sound<br />"
                                                                "section of the settings tab. Also make sure your version of <br />"
                                                                "qemu/KVM has support for the sound system you selected.")
                                                                ,QMessageBox::Ok);
        return;
    }
    QMessageBox::critical(this, tr("QtEmu Error"), tr("An error has occurred. This may have been caused by<br />"
                                                      "an incorrect setting. The error is:<br />") + errorMsg,QMessageBox::Ok);
}

void MachineTab::snapshot(const int state)
{
    if(state == Qt::Unchecked)
    {
        snapshotCheckBox->setText(tr("Snapshot mode"));
    }
}

void MachineTab::runCommand()
{
    machineProcess->write(consoleCommand->text().toAscii() + '\n');
    consoleCommand->clear();
}

void MachineTab::restart()
{
    connect(machineProcess, SIGNAL(finished(int)) , this, SLOT(clearRestart()));
    machineProcess->stop();
}

void MachineTab::clearRestart()
{
    disconnect(machineProcess, SIGNAL(finished(int)) , this, SLOT(clearRestart()));
    startButton->click();
    
}

void MachineTab::booting()
{
    if(machineConfigObject->getOption("embeddedDisplay",QVariant(false)).toBool())
        machineView->initView();
    if(snapshotCheckBox->isChecked())
    {
       snapshotCheckBox->setText(snapshotCheckBox->text() + "\n" + tr("(uncheck to commit changes)"));
    }
    pauseButton->setEnabled(true);
    suspendButton->setHidden(false);
    resumeButton->setHidden(true);
}

void MachineTab::cleanupView()
{
    machineView->showSplash(true);
}

void MachineTab::takeScreenshot()
{
    //TODO: for now just save a screenshot to the preview location.
    // should provide a save dialog and have a dropdown to save as the preview.
    QString fileName = machineConfigObject->getOption("hdd",QString()).toString().replace(QRegExp("[.][^.]+$"), ".ppm");
    machineProcess->write(QString("screendump " + fileName).toAscii() + '\n');
    machineConfigObject->setOption("preview", fileName);
}

void MachineTab::makeConnections()
{
    connect(machineNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(QString)));

    //hard disk manager related connections
    HardDiskManager *hdManager = machineProcess->getHdManager();
    connect(settingsTab, SIGNAL(upgradeHdd()), hdManager, SLOT(upgradeImage()));
    connect(hdManager, SIGNAL(imageUpgradable(bool)), settingsTab->upgradeFrame, SLOT(setEnabled(bool)));
    connect(hdManager, SIGNAL(processingImage(bool)), this, SLOT(setDisabled(bool)));
    connect(hdManager, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));
    connect(hdManager, SIGNAL(imageFormat(QString)), settingsTab->formatLabel, SLOT(setText(QString)));
    connect(hdManager, SIGNAL(imageSize(qint64)), settingsTab, SLOT(setVirtSize(qint64)));
    connect(hdManager, SIGNAL(phySize(qint64)), settingsTab, SLOT(setPhySize(qint64)));

    connect(hdManager, SIGNAL(supportsSuspending(bool)), suspendButton, SLOT(setEnabled(bool)));
    connect(hdManager, SIGNAL(supportsResuming(bool)), resumeButton, SLOT(setEnabled(bool)));
}
