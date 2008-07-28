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
    machineConfigObject->setOption("vncPort", 1000 + parentTabWidget->currentIndex() + 1);
    //this is a hack... eventually we will be able to support using a unix socket (a file) instead of a network port.

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
    connect(machineNameEdit, SIGNAL(textChanged(QString)), machineProcess, SLOT(name(QString)));

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
    connect(snapshotCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(snapshot(int)));
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
    
    //TODO: add a fullscreen button here
    
    QLabel *notesLabel = new QLabel(tr("<strong>Notes</strong>"), this);

    notesTextEdit = new QTextEdit(this);
    notesTextEdit->setFixedHeight(60);
#if QT_VERSION >= 0x040200
    notesTextEdit->setStyleSheet(QString(flatStyle).replace("TYPE", "QTextEdit"));
#endif
    QLabel *devicesLabel = new QLabel(tr("<strong>Devices</strong>"), this);

    QVBoxLayout *devicesLayout = new QVBoxLayout;

    //memory section start
    memoryButton = new QPushButton(QIcon(":/images/" + iconTheme + "/memory.png"), tr("&Memory"), this);
    memoryButton->setCheckable(true);
    devicesLayout->addWidget(memoryButton);

    memoryFrame = new QFrame(this);
    memoryFrame->setVisible(false);
    devicesLayout->addWidget(memoryFrame);
    connect(memoryButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(memoryButton, SIGNAL(clicked(bool)), memoryFrame, SLOT(setVisible(bool)));
    connect(memoryButton, SIGNAL(clicked(bool)), memoryButton, SLOT(setChecked(bool)));

    QVBoxLayout *memoryFrameLayout = new QVBoxLayout;
    memoryFrame->setLayout(memoryFrameLayout);

    QLabel *memoryDescriptionLabel = new QLabel(tr("Set the size of memory for this virtual machine."
                                                    " If you set too high an amount, memory "
                                                    "swapping may occur.<br /><br />"
                                                    "Memory for this virtual machine:"), this);
    memoryDescriptionLabel->setWordWrap(true);
    memoryFrameLayout->addWidget(memoryDescriptionLabel);

    memorySlider = new QSlider(Qt::Horizontal, this);
    memorySlider->setRange(4, 1024);
    memorySlider->setSingleStep(4);
    memorySlider->setTickPosition(QSlider::TicksBelow);
    memorySlider->setTickInterval(64);

    QSpinBox *memorySpinBox = new QSpinBox(this);
    memorySpinBox->setRange(4, 1024);
    memorySpinBox->setSingleStep(4);

    QLabel *memoryMbLabel = new QLabel(tr("MB"), this);

    connect(memorySlider, SIGNAL(valueChanged(int)), memorySpinBox, SLOT(setValue(int)));
    connect(memorySpinBox, SIGNAL(valueChanged(int)), memorySlider, SLOT(setValue(int)));

    connect(memorySpinBox, SIGNAL(valueChanged(int)), machineProcess, SLOT(memory(int)));

    QHBoxLayout *memoryLayout = new QHBoxLayout;
    memoryLayout->addWidget(memorySlider);
    memoryLayout->addWidget(memorySpinBox);
    memoryLayout->addWidget(memoryMbLabel);

    memoryFrameLayout->addLayout(memoryLayout);

    QLabel *cpuDescriptionLabel = new QLabel(tr("<hr>&Number of virtual CPUs:"), this);
    cpuDescriptionLabel->setWordWrap(true);
    memoryFrameLayout->addWidget(cpuDescriptionLabel);

    cpuSpinBox = new QSpinBox(this);
    cpuSpinBox->setRange(1, 4);
    cpuSpinBox->setValue(2);
    cpuDescriptionLabel->setBuddy(cpuSpinBox);
    connect(cpuSpinBox, SIGNAL(valueChanged(int)), machineProcess, SLOT(cpu(int)));

    QLabel *cpuLabel = new QLabel(tr("Virtual CPU(s)"), this);

    virtualizationCheckBox = new QCheckBox(tr("Enable &virtualization"), this);
    connect(virtualizationCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(virtualization(int)));

    QHBoxLayout *cpuLayout = new QHBoxLayout;
    cpuLayout->addWidget(cpuSpinBox);
    cpuLayout->addWidget(cpuLabel);
    cpuLayout->addWidget(virtualizationCheckBox);
    cpuLayout->addStretch();
    memoryFrameLayout->addLayout(cpuLayout);
    //memory section end

    //hdd section start
    hddButton = new QPushButton(QIcon(":/images/" + iconTheme + "/hdd.png"), tr("&Hard Disk"), this);
    hddButton->setCheckable(true);
    devicesLayout->addWidget(hddButton);

    hddFrame = new QFrame(this);
    hddFrame->setVisible(false);
    devicesLayout->addWidget(hddFrame);
    connect(hddButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(hddButton, SIGNAL(clicked(bool)), hddFrame, SLOT(setVisible(bool)));
    connect(hddButton, SIGNAL(clicked(bool)), hddButton, SLOT(setChecked(bool)));

    QVBoxLayout *hddFrameLayout = new QVBoxLayout;
    hddFrame->setLayout(hddFrameLayout);

    QLabel *hddDescriptionLabel = new QLabel(tr("Select a valid hard disk image for QtEmu. "
                                                "Do <b>not change</b> the hard disk image unless you "
                                                "know what you are doing!<br /><br />"
                                                "Hard disk image for this virtual machine:"), this);
    hddDescriptionLabel->setWordWrap(true);
    hddFrameLayout->addWidget(hddDescriptionLabel);

    hddPathLineEdit = new QLineEdit(this);
    connect(hddPathLineEdit, SIGNAL(textChanged(QString)), machineProcess, SLOT(path(QString)));
    connect(hddPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(testHDDImage(QString)));
    connect(hddPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(updatePreview(QString)));

    QPushButton *hddSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/open.png"), QString(), this);
    connect(hddSelectButton, SIGNAL(clicked()), this, SLOT(setNewHddPath()));

    hddUpgradeButton = new QPushButton(QIcon(":/images/" + iconTheme + "/wizard.png"), QString(tr("Upgrade HDD Format to Native")), this);
    connect(hddUpgradeButton, SIGNAL(clicked()), this, SLOT(upgradeImage()));

    QHBoxLayout *hddLayout = new QHBoxLayout;
    hddLayout->addWidget(hddPathLineEdit);
    hddLayout->addWidget(hddSelectButton);

    hddFrameLayout->addLayout(hddLayout);
    hddFrameLayout->addWidget(hddUpgradeButton);
    hddUpgradeButton->setHidden(true);
    //hdd section end

    //cdrom section start
    cdromButton = new QPushButton(QIcon(":/images/" + iconTheme + "/cdrom.png"), tr("&CD ROM"), this);
    cdromButton->setCheckable(true);
    devicesLayout->addWidget(cdromButton);

    cdromFrame = new QFrame(this);
    cdromFrame->setVisible(false);
    devicesLayout->addWidget(cdromFrame);
    connect(cdromButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(cdromButton, SIGNAL(clicked(bool)), cdromFrame, SLOT(setVisible(bool)));
    connect(cdromButton, SIGNAL(clicked(bool)), cdromButton, SLOT(setChecked(bool)));

    QVBoxLayout *cdromFrameLayout = new QVBoxLayout;
    cdromFrame->setLayout(cdromFrameLayout);

    QLabel *cdromDescriptionLabel = new QLabel(tr("Select a valid CD ROM image or a physical device.<br /><br />"
                                                  "Image or device for this virtual machine:"), this);
    cdromDescriptionLabel->setWordWrap(true);
    cdromFrameLayout->addWidget(cdromDescriptionLabel);

    cdromLineEdit = new QLineEdit(this);
    connect(cdromLineEdit, SIGNAL(textChanged(const QString&)), machineProcess, SLOT(cdRomPath(const QString&)));

    QPushButton *cdromSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/cdrom.png"), QString(), this);
    cdromSelectButton->setToolTip(tr("Select a CD ROM Drive"));
    connect(cdromSelectButton, SIGNAL(clicked()), this, SLOT(setNewCdRomPath()));

    QPushButton *cdImageSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/cdimage.png"), QString(), this);
    cdImageSelectButton->setToolTip(tr("Select a CD Image"));
    connect(cdImageSelectButton, SIGNAL(clicked()), this, SLOT(setNewCdImagePath()));

    QHBoxLayout *cdromLayout = new QHBoxLayout;
    cdromLayout->addWidget(cdromLineEdit);
    cdromLayout->addWidget(cdromSelectButton);
    cdromLayout->addWidget(cdImageSelectButton);
    cdromFrameLayout->addLayout(cdromLayout);

    cdBootCheckBox = new QCheckBox(tr("&Boot from CD ROM"), this);
    connect(cdBootCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(cdBoot(int)));
    cdromReloadButton = new QPushButton(tr("Reload the virtual CD &ROM"));
    
    connect(cdromReloadButton, SIGNAL(clicked()), machineProcess, SLOT(changeCdrom()));
    
    cdromFrameLayout->addWidget(cdromReloadButton);
    cdromFrameLayout->addWidget(cdBootCheckBox);
    //cdrom section end

    //floppy section start
    floppyButton = new QPushButton(QIcon(":/images/" + iconTheme + "/floppy.png"), tr("&Floppy Disk"), this);
    floppyButton->setCheckable(true);
    devicesLayout->addWidget(floppyButton);

    floppyFrame = new QFrame(this);
    floppyFrame->setVisible(false);
    devicesLayout->addWidget(floppyFrame);
    connect(floppyButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(floppyButton, SIGNAL(clicked(bool)), floppyFrame, SLOT(setVisible(bool)));
    connect(floppyButton, SIGNAL(clicked(bool)), floppyButton, SLOT(setChecked(bool)));

    QVBoxLayout *floppyFrameLayout = new QVBoxLayout;
    floppyFrame->setLayout(floppyFrameLayout);

    QLabel *floppyDescriptionLabel = new QLabel(tr("Select a valid floppy disk image or a physical device.<br /><br />"
                                                   "Image or device for this virtual machine:"), this);
    floppyDescriptionLabel->setWordWrap(true);
    floppyFrameLayout->addWidget(floppyDescriptionLabel);

    floppyLineEdit = new QLineEdit(this);
    connect(floppyLineEdit, SIGNAL(textChanged(const QString&)), machineProcess, SLOT(floppyDiskPath(const QString&)));

    QPushButton *floppySelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/floppy.png"), QString(), this);
    floppySelectButton->setToolTip(tr("Select a Floppy Disk Drive"));
    connect(floppySelectButton, SIGNAL(clicked()), this, SLOT(setNewFloppyDiskPath()));

    QPushButton *floppyImageSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/cdimage.png"), QString(), this);
    floppyImageSelectButton->setToolTip(tr("Select a Floppy Disk Image"));
    connect(floppyImageSelectButton, SIGNAL(clicked()), this, SLOT(setNewFloppyDiskImagePath()));

    QHBoxLayout *floppyLayout = new QHBoxLayout;
    floppyLayout->addWidget(floppyLineEdit);
    floppyLayout->addWidget(floppySelectButton);
    floppyLayout->addWidget(floppyImageSelectButton);
    floppyFrameLayout->addLayout(floppyLayout);

    floppyBootCheckBox = new QCheckBox(tr("&Boot from floppy disk"), this);
    connect(floppyBootCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(floppyBoot(int)));
    
    floppyReloadButton = new QPushButton(tr("Reload the virtual Floppy Disk"));
    
    connect(floppyReloadButton, SIGNAL(clicked()), machineProcess, SLOT(changeFloppy()));
    
    floppyFrameLayout->addWidget(floppyReloadButton);
    floppyFrameLayout->addWidget(floppyBootCheckBox);
    //floppy section end

    //network section start
    networkButton = new QPushButton(QIcon(":/images/" + iconTheme + "/network.png"), tr("&Network"), this);
    networkButton->setCheckable(true);
    devicesLayout->addWidget(networkButton);

    networkFrame = new QFrame(this);
    networkFrame->setVisible(false);
    devicesLayout->addWidget(networkFrame);
    connect(networkButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(networkButton, SIGNAL(clicked(bool)), networkFrame, SLOT(setVisible(bool)));
    connect(networkButton, SIGNAL(clicked(bool)), networkButton, SLOT(setChecked(bool)));

    QVBoxLayout *networkFrameLayout = new QVBoxLayout;
    networkFrame->setLayout(networkFrameLayout);

    QLabel *networkDescriptionLabel = new QLabel(tr("Choose whether the network (and internet) connection should "
                                                    "be available for this virtual machine. Different network "
                                                    "modes are available, and multiple modes can be used at once."), this);
    networkDescriptionLabel->setWordWrap(true);
    networkFrameLayout->addWidget(networkDescriptionLabel);

    networkCheckBox = new QCheckBox(tr("&Enable network"), this);
    networkCheckBox->setToolTip(tr("By default, the Qtemu uses User Mode Networking.\n"
                                    "This default should be fine for most people.\n"));
    connect(networkCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(network(int)));
    connect(networkCheckBox, SIGNAL(stateChanged(int)), this, SLOT(network(int)));
    networkFrameLayout->addWidget(networkCheckBox);
    
    networkDescriptionLabel = new QLabel(tr("Choose a folder to use as a virtual network drive:"), this);
    
    networkDescriptionLabel->setWordWrap(true);
    networkFrameLayout->addWidget(networkDescriptionLabel);
    smbFolderEdit = new QLineEdit(this);
    networkDescriptionLabel->setBuddy(smbFolderEdit);
    connect(smbFolderEdit, SIGNAL(textChanged(const QString&)), machineProcess, SLOT(smbFolderPath(const QString&)));

    QPushButton *smbSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/network.png"), QString(), this);
    smbSelectButton->setToolTip(tr("Select a Folder to share"));
    connect(smbSelectButton, SIGNAL(clicked()), this, SLOT(setNewSmbFolderPath()));

    QHBoxLayout *smbLayout = new QHBoxLayout;
    smbLayout->addWidget(smbFolderEdit);
    smbLayout->addWidget(smbSelectButton);
    networkFrameLayout->addLayout(smbLayout);
    networkDescriptionLabel->setBuddy(smbFolderEdit);
    
    networkDescriptionLabel = new QLabel(tr("Advanced Network Modes:"), this);
    networkDescriptionLabel->setWordWrap(true);
    networkFrameLayout->addWidget(networkDescriptionLabel);

    
    QCheckBox *userModeNetwork = new QCheckBox(tr("User mode networking"));
    userModeNetwork->setToolTip(tr("In this mode the virtual machine accesses the network\n"
                                   "using Slirp; this is similar to access with a  web\n"
                                   "browser. this mode does not require administrator access,\n"
                                   "and works with wireless cards."));
    connect(userModeNetwork, SIGNAL(toggled(bool)), this, SLOT(unimplemented()));
    
    QCheckBox *bridgedModeNetwork = new QCheckBox(tr("Bridged networking"));
    bridgedModeNetwork->setToolTip(tr("In this mode the virtual machine will have direct\n"
                                      "access to the host's network; This is needed to allow\n"
                                      "ICMP (ping) to work, and allows other machines to 'see'\n"
                                      "your virtual machine on the network. This mode does not\n"
                                      "work with most wireless cards, and may cause problems\n"
                                      "for NetworkManager."));//TODO: automatically disable NetworkManager and re-enable after VM shutdown, via d-bus
    connect(bridgedModeNetwork, SIGNAL(toggled(bool)), this, SLOT(unimplemented()));
    
    QCheckBox *localBridgedModeNetwork = new QCheckBox(tr("Local bridged networking"));
    localBridgedModeNetwork->setToolTip(tr("This mode allows more advanced bridging techniques,\n"
                                          "including using the host computer as a router or\n"
                                          "restricting access to the host machine only.\n"
                                          "This is safe with NetworkManager."));
    connect(localBridgedModeNetwork, SIGNAL(toggled(bool)), this, SLOT(unimplemented()));
/*    
    QCheckBox *sharedVlanNetwork = new QCheckBox(tr("Shared VLan Networking"));
    sharedVlanNetwork->setToolTip(tr("This mode adds a network that is shared exclusively\n"
                                     "between virtual machines. IP based guests will default\n"
                                     "to APIPA addresses unless you run a DHCP server on\n"
                                     "one of your virtual machines. This does not use bridging."));
    connect(sharedVlanNetwork, SIGNAL(toggled(bool)), this, SLOT(unimplemented()));
*/    
    networkFrameLayout->addWidget(userModeNetwork);
    networkFrameLayout->addWidget(bridgedModeNetwork);
    networkFrameLayout->addWidget(localBridgedModeNetwork);
//    networkFrameLayout->addWidget(sharedVlanNetwork);
    
    networkDescriptionLabel = new QLabel(tr("Custom Networking Options:"), this);
    networkDescriptionLabel->setWordWrap(true);
    networkFrameLayout->addWidget(networkDescriptionLabel);
    networkCustomOptionsEdit = new QLineEdit(this);
    networkFrameLayout->addWidget(networkCustomOptionsEdit);
    

    connect(networkCustomOptionsEdit, SIGNAL(textChanged(const QString&)),
            machineProcess, SLOT(networkCustomOptions(const QString&)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), networkDescriptionLabel, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), networkCustomOptionsEdit, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), userModeNetwork, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), bridgedModeNetwork, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), localBridgedModeNetwork, SLOT(setEnabled(bool)));
//    connect(networkCheckBox, SIGNAL(toggled(bool)), sharedVlanNetwork, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), smbFolderEdit, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), smbSelectButton, SLOT(setEnabled(bool)));
    //network section end

    //sound section start
    soundButton = new QPushButton(QIcon(":/images/" + iconTheme + "/sound.png"), tr("&Sound && Video"), this);
    soundButton->setCheckable(true);
    devicesLayout->addWidget(soundButton);

    soundFrame = new QFrame(this);
    soundFrame->setVisible(false);
    devicesLayout->addWidget(soundFrame);
    connect(soundButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(soundButton, SIGNAL(clicked(bool)), soundFrame, SLOT(setVisible(bool)));
    connect(soundButton, SIGNAL(clicked(bool)), soundButton, SLOT(setChecked(bool)));

    QVBoxLayout *soundFrameLayout = new QVBoxLayout;
    soundFrame->setLayout(soundFrameLayout);

    videoCheckBox = new QCheckBox(tr("Enable the embedded display"), this);
    machineConfigObject->registerObject(videoCheckBox,"embeddedDisplay", Qt::Checked);

    videoResizeCheckBox = new QCheckBox(tr("Scale display to fit window"), this);
    
    machineConfigObject->registerObject(videoResizeCheckBox,"scaleEmbeddedDisplay", Qt::Checked);

    QLabel *soundDescriptionLabel = new QLabel(tr("<hr>Choose whether sound support should "
                                                  "be available for this virtual machine."), this);
    soundDescriptionLabel->setWordWrap(true);
    
    soundCheckBox = new QCheckBox(tr("&Enable sound"), this);
    QLabel *soundSystemDescriptionLabel = new QLabel(tr("Choose sound system to use for sound emulation."), this);
    soundSystemDescriptionLabel->setWordWrap(true);
    soundSystemGroup = new QButtonGroup();
    soundALSARadioButton = new QRadioButton(tr("Use &ALSA"), this);
    soundOSSRadioButton = new QRadioButton(tr("Use &OSS"), this);
    soundESDRadioButton = new QRadioButton(tr("Use &ESD"), this);
    soundSystemGroup->addButton(soundALSARadioButton, 1);
    soundSystemGroup->addButton(soundOSSRadioButton, 2);
    soundSystemGroup->addButton(soundESDRadioButton, 3);




    connect(soundCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(sound(int)));
    connect(soundSystemGroup, SIGNAL(buttonClicked(int)), this, SLOT(setSoundSystem(int)));
    soundOSSRadioButton->click();
    soundFrameLayout->addWidget(videoCheckBox);
    soundFrameLayout->addWidget(videoResizeCheckBox);
    soundFrameLayout->addWidget(soundDescriptionLabel);
    soundFrameLayout->addWidget(soundCheckBox);
    soundFrameLayout->addWidget(soundSystemDescriptionLabel);

    soundFrameLayout->addWidget(soundALSARadioButton);
    soundFrameLayout->addWidget(soundOSSRadioButton);
    soundFrameLayout->addWidget(soundESDRadioButton);
    //sound section end


    //other section start
    otherButton = new QPushButton(QIcon(":/images/" + iconTheme + "/other.png"), tr("&Other"), this);
    otherButton->setCheckable(true);
    devicesLayout->addWidget(otherButton);

    otherFrame = new QFrame(this);
    otherFrame->setVisible(false);
    devicesLayout->addWidget(otherFrame);
    connect(otherButton, SIGNAL(clicked(bool)), this, SLOT(closeAllSections()));
    connect(otherButton, SIGNAL(clicked(bool)), otherFrame, SLOT(setVisible(bool)));
    connect(otherButton, SIGNAL(clicked(bool)), otherButton, SLOT(setChecked(bool)));

    QVBoxLayout *otherFrameLayout = new QVBoxLayout;
    otherFrame->setLayout(otherFrameLayout);

    QLabel *mouseDescriptionLabel = new QLabel(tr("Choose whether the mouse should switch seamlessly between "
                                                  "host and virtual system."), this);
    mouseDescriptionLabel->setWordWrap(true);
    otherFrameLayout->addWidget(mouseDescriptionLabel);

    mouseCheckBox = new QCheckBox(tr("Enable seamless mo&use"), this);
    mouseCheckBox->setToolTip(tr("This option depends on the operating system. It is not supported by non-graphical "
                                 "systems. <strong>Attention:</strong> This option may reduce the system performance." ));
    connect(mouseCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(mouse(int)));
    otherFrameLayout->addWidget(mouseCheckBox);

    QLabel *timeDescriptionLabel = new QLabel(tr("<hr>Choose if the virtual machine should use "
                                                 "the host machine clock."), this);
    timeDescriptionLabel->setWordWrap(true);
    otherFrameLayout->addWidget(timeDescriptionLabel);

    timeCheckBox = new QCheckBox(tr("Enable &local time"), this);
    connect(timeCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(time(int)));
    otherFrameLayout->addWidget(timeCheckBox);

    additionalOptionsCheckBox = new QCheckBox(tr("&Additional QEMU options:"), this);
    connect(additionalOptionsCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(useAdditionalOptions(int)));
    otherFrameLayout->addWidget(new QLabel(QLatin1String("<hr>"), this));
    otherFrameLayout->addWidget(additionalOptionsCheckBox);

    additionalOptionsEdit = new QLineEdit(this);
    otherFrameLayout->addWidget(additionalOptionsEdit);
    connect(additionalOptionsEdit, SIGNAL(textChanged(const QString&)),
            machineProcess, SLOT(additionalOptions(const QString&)));
    connect(additionalOptionsCheckBox, SIGNAL(toggled(bool)),
            additionalOptionsEdit, SLOT(setEnabled(bool)));

    //other section end

    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->addLayout(closeButtonLayout);
    buttonsLayout->addLayout(powerButtonsLayout);
    buttonsLayout->addLayout(controlButtonsLayout);
    buttonsLayout->addLayout(snapshotLayout);
    buttonsLayout->addWidget(notesLabel);
    buttonsLayout->addWidget(notesTextEdit);
    buttonsLayout->addWidget(devicesLabel);
    buttonsLayout->addLayout(devicesLayout);
    buttonsLayout->addStretch();

    //set up the layout for the tab panel
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addLayout(buttonsLayout, 0, 0);
    
    QTabWidget *viewTabs = new QTabWidget(this);
    mainLayout->addWidget(viewTabs, 0, 1);   
    mainLayout->setColumnStretch(1, 10);

    viewFrame = new QFrame(this);
    viewTabs->addTab(viewFrame, tr("Display"));


    machineView = new MachineView(this);
    machineConfigObject->registerObject(machineView);
    viewLayout = new QGridLayout();
    viewFrame->setLayout(viewLayout); 
    viewLayout->setColumnStretch(1, 10);
    viewLayout->setRowStretch(1, 10);
    viewLayout->addWidget(machineView, 1, 1);


    settingsFrame = new QFrame(this);
    viewTabs->addTab(settingsFrame, tr("Settings"));
    
    consoleFrame = new QFrame(this);
    viewTabs->addTab(consoleFrame, tr("Console"));
    
    console = new QTextEdit(this);
    console->setReadOnly(true);
    connect(machineProcess, SIGNAL(cleanConsole(QString)), console, SLOT(append(QString)));
    consoleCommand = new QLineEdit(this);
    console->setFocusProxy(consoleCommand);
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
    
    read();

    //read first the name, otherwise the name of the main tab changes
    connect(machineNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(QString)));

}

//TODO: the functionality in here really should be abstracted into another class, like MachineImage
void MachineTab::testHDDImage(const QString &path)
{
    QFileInfo *currentImage = new QFileInfo(path);
    
    if(currentImage->suffix()!="qcow")
    {
        hddUpgradeButton->setHidden(false);
        suspendButton->setEnabled(false);
        resumeButton->setEnabled(false);
    }
    else
    {
        hddUpgradeButton->setHidden(true);
        suspendButton->setEnabled(true);

        //test for a valid suspend/resume image
        QProcess *testImage = new QProcess();
        QStringList arguments;
        arguments <<"info" << path;
        testImage->start("qemu-img", arguments);
        testImage->waitForFinished();
        QString output = testImage->readAll();
        if(output.contains("Default"))
            resumeButton->setEnabled(true);
        else
            resumeButton->setEnabled(false);
    }
}

//TODO: the functionality in here really should be abstracted into another class, like MachineImage
void MachineTab::upgradeImage()
{
    if (QMessageBox::question(this, tr("Upgrade Confirmation"),
                              tr("This will upgrade your Hard Disk image to the qcow format.<br />This enables more advanced features such as suspend/resume on all operating systems and image compression on Windows.<br />Your old image will remain intact, so if you want to revert afterwards you may do so."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
      == QMessageBox::Yes)
    {
        QFileInfo *currentImage = new QFileInfo(hddPathLineEdit->text());
        QString finalName = currentImage->path() + '/' + currentImage->completeBaseName() + ".qcow";
        QStringList arguments;
        QProcess *upgradeProcess = new QProcess(this);
        connect(upgradeProcess, SIGNAL(started()), this, SLOT(upgradeImageStarted()));
        connect(upgradeProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(upgradeImageFinished(int)));
        
        arguments << "convert" << currentImage->filePath() << "-O" << "qcow2" << finalName;
        QString program = "qemu-img";
        upgradeProcess->start(program, arguments);
    }
}

//TODO: the functionality in here really should be abstracted into another class, like DiskImage
void MachineTab::upgradeImageStarted()
{
    startButton->setEnabled(false);
    resumeButton->setEnabled(false);
    
    hddUpgradeButton->setEnabled(false);
    hddPathLineEdit->setEnabled(false);
    hddUpgradeButton->setText(tr("Upgrading..."));
}

void MachineTab::upgradeImageFinished(const int &exitCode)
{
    QFileInfo *oldImage = new QFileInfo(hddPathLineEdit->text());
    QFileInfo *newImage = new QFileInfo(oldImage->path() + '/' + oldImage->completeBaseName() + ".qcow");
    hddPathLineEdit->setEnabled(true);
    if(newImage->exists()&&exitCode==0)
    {
        hddPathLineEdit->setText(newImage->filePath());
        QMessageBox::information(window(), tr("Upgrade Complete"),
                                   tr("Upgrade complete. Your old hard disk image is preserved.<br />After you have determined the upgrade went smoothly and your machine will still start, you may wish to delete the old image."));
    }
    else
    {
    QMessageBox::warning(window(), tr("Upgrade Failed"),
                                   tr("Upgrading your hard disk image failed! Do you have enough disk space?<br />You may want to try upgrading manually using the program qemu-img."));
    }
    startButton->setEnabled(true);
    hddUpgradeButton->hide();
    hddUpgradeButton->setText(tr("Upgrade HDD Format to Native"));
}

void MachineTab::setNewHddPath()
{
    QString newHddPath = QFileDialog::getOpenFileName(this, tr("Select a QtEmu hard disk image"),
                                              myMachinesPath,
                                              tr("QtEmu hard disk images")+" (*.img *.qcow *.vmdk)");
    if (!newHddPath.isEmpty())
        hddPathLineEdit->setText(newHddPath);
}

void MachineTab::setNewCdRomPath()
{
  //TODO: a dialog that displays current removable drives and allows one to be selected
  //instead of a file picker.
  // /sys/block/<kernel device>/device/uevent contains "DRIVER=ide-cdrom" or "DRIVER=sr" if it is optical, drive is at /dev/<kernel device>.
  // the box should show the model name of the device, or perhaps just "Internal Optical Drive"
  //if there is only one drive in the PC. 
  //the device name is available in /proc/ide/<kernel device>/model for ide (hdx) drives
  //and at /sys/block/<kernel device>/device/model for scsi (sdx) drives.
  
#ifdef Q_OS_WIN32
    QMessageBox::warning(window(), tr("QtEmu"),
                                   tr("This function is not available under Windows due to the missing function "
                                      "of QEMU under Windows. It will probably be fixed in a later version."));
    return;
#endif
    QString newCdPath = QFileDialog::getOpenFileName(this, tr("Select a CD ROM Drive"),
                                                          cdromLineEdit->text(), "");
    if (!newCdPath.isEmpty())
        cdromLineEdit->setText(newCdPath);
}

void MachineTab::setNewCdImagePath()
{
    QString newCdPath = QFileDialog::getOpenFileName(this, tr("Select a CD Image"),
                                                     myMachinesPath,
                                                     tr("CD ROM images")+" (*.iso *.img)");
    if (!newCdPath.isEmpty())
        cdromLineEdit->setText(newCdPath);
}

void MachineTab::setNewFloppyDiskPath()
{
#ifdef Q_OS_WIN32
    QMessageBox::warning(window(), tr("QtEmu"),
                                   tr("This function is not available under Windows due to the missing function "
                                      "of QEMU under Windows. It will probably be fixed in a later version."));
    return;
#endif
    QString newFloppyPath = QFileDialog::getExistingDirectory(this, tr("Select a Floppy Disk Drive"),
                                                              myMachinesPath);
    if (!newFloppyPath.isEmpty())
        floppyLineEdit->setText(newFloppyPath);
}

void MachineTab::setNewFloppyDiskImagePath()
{
    QString newFloppyPath = QFileDialog::getOpenFileName(this, tr("Select a Floppy Disk Image"),
                                                         myMachinesPath,
                                                         tr("Floppy disk images")+" (*.iso *.img)");
    if (!newFloppyPath.isEmpty())
        floppyLineEdit->setText(newFloppyPath);
}

void MachineTab::closeAllSections()
{
    memoryButton->setChecked(false);
    memoryFrame->setVisible(false);
    hddButton->setChecked(false);
    hddFrame->setVisible(false);
    cdromButton->setChecked(false);
    cdromFrame->setVisible(false);
    floppyButton->setChecked(false);
    floppyFrame->setVisible(false);
    networkButton->setChecked(false);
    networkFrame->setVisible(false);
    soundButton->setChecked(false);
    soundFrame->setVisible(false);
    otherButton->setChecked(false);
    otherFrame->setVisible(false);
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
    machineConfigObject->registerObject(hddPathLineEdit, "hdd");
    machineConfigObject->registerObject(memorySlider, "memory");
    machineConfigObject->registerObject(cdromLineEdit, "cdrom");
    machineConfigObject->registerObject(cdBootCheckBox, "bootFromCd", QVariant(false));
    machineConfigObject->registerObject(floppyLineEdit, "floppy");
    machineConfigObject->registerObject(floppyBootCheckBox, "bootFromFloppy", QVariant(false));
    machineConfigObject->registerObject(networkCheckBox, "network", QVariant(true));
    machineConfigObject->registerObject(soundCheckBox, "sound", QVariant(false));
    machineConfigObject->registerObject(soundSystemGroup, "soundSystem", QVariant(soundOSSRadioButton->text()));
    
    machineConfigObject->registerObject(networkCustomOptionsEdit, "networkCustomOptions");
    machineConfigObject->registerObject(mouseCheckBox, "mouse", QVariant(true));
    machineConfigObject->registerObject(timeCheckBox, "time", QVariant(true));
    machineConfigObject->registerObject(virtualizationCheckBox, "virtualization", QVariant(false));
    machineConfigObject->registerObject(cpuSpinBox, "cpu", QVariant(1));
    machineConfigObject->registerObject(additionalOptionsEdit, "additionalOptions");
    machineConfigObject->registerObject(additionalOptionsCheckBox, "useAdditionalOptions", QVariant(false));
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
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    machineProcess->start();
    console->clear();
}

void MachineTab::suspending()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    suspendButton->setEnabled(false);
    pauseButton->setEnabled(false);
    suspendButton->setText(tr("Suspending..."));
}

void MachineTab::suspended()
    {
    machineProcess->forceStop();
    resumeButton->setHidden(false);
    
    testHDDImage(hddPathLineEdit->text());//will enable the resume button if appropriate
    //resumeButton->setEnabled(true);
    suspendButton->setHidden(true);
    suspendButton->setText(tr("&Suspend"));
    }

void MachineTab::resuming()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
}

void MachineTab::resumed()
{
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
                             "You should only do this if the machine is unresponsive. Doing this may cause damage to the disk image."));
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QPushButton *forceShutdownButton = msgBox.addButton(tr("Force Shutdown"), QMessageBox::DestructiveRole);
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
    hddUpgradeButton->setEnabled(true);
    snapshotCheckBox->setText(tr("Snapshot mode"));
    cleanupView();
}

void MachineTab::started()
{
    if(snapshotCheckBox->isChecked())
    {
       snapshotCheckBox->setText(snapshotCheckBox->text() + tr("(uncheck to commit changes)"));
    }
    pauseButton->setEnabled(true);
    suspendButton->setHidden(false);
    resumeButton->setHidden(true);
    hddUpgradeButton->setEnabled(false);
}

void MachineTab::error(const QString & errorMsg)
{
    QMessageBox::critical(this, tr("QtEmu Error"), tr("An error has occurred in qemu. This may have been caused by QtEmu<br />"
                                                      "or by attempting to perform an invalid action. The error is:<br />") + errorMsg,QMessageBox::Ok);
}

void MachineTab::snapshot(const int state)
{
    if(state == Qt::Unchecked)
    {
        snapshotCheckBox->setText(tr("Snapshot mode"));
    }
}

void MachineTab::setNewSmbFolderPath()
{
#ifdef Q_OS_WIN32
    QMessageBox::warning(window(), tr("QtEmu"),
                                   tr("This function is not available under Windows."
                                      "You should be able to use Windows file sharing to enable it manually, however."));
    return;
#endif
    QString newSmbPath = QFileDialog::getExistingDirectory(this, tr("Select a folder to use as a Virtual Network Drive"),
                                                          myMachinesPath);
    if (!newSmbPath.isEmpty())
        smbFolderEdit->setText(newSmbPath);
}

void MachineTab::network(const int value)
{
}

void MachineTab::unimplemented()
{
    QMessageBox::warning(window(), tr("QtEmu"),
                                   tr("This function is not yet implemented."));
}

void MachineTab::supressAudioErrors()
{
    machineProcess->supressError("oss");
    machineProcess->supressError("audio");
}

void MachineTab::setSoundSystem(int id)
{
    switch(id)
    {
        case 1:
            machineProcess->soundSystem("alsa");
            break;
        case 2:
            machineProcess->soundSystem("oss");
            break;
        case 3:
            machineProcess->soundSystem("esd");
            break;
    }
}

void MachineTab::runCommand()
{
    machineProcess->write(consoleCommand->text().toAscii() + "\n");
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
    machineView->machineNumber(parentTabWidget->indexOf(this));
    if(videoCheckBox->checkState() == Qt::Checked)
        machineView->initView();
}

void MachineTab::cleanupView()
{
    machineView->showSplash(true);
}

void MachineTab::updatePreview(const QString & hdPath)
{
    machineView->setPreview(hdPath + ".ppm");
}

void MachineTab::takeScreenshot()
{
    //TODO: for now just save a screenshot to the preview location.
    // should provide a save dialog and have a dropdown to save as the preview.
    
    machineProcess->write(QString("screendump " + hddPathLineEdit->text() + ".ppm").toAscii() + '\n');
}

