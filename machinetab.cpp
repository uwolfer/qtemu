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
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QFileInfo>
#include <QMenu>

MachineTab::MachineTab(QTabWidget *parent, const QString &fileName, const QString &myMachinesPathParent)
    : QWidget(parent)
{
    parentTabWidget = parent;
    xmlFileName = fileName;
    myMachinesPath = myMachinesPathParent;

    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();

    machineProcess = new MachineProcess(this);
    
    connect(machineProcess, SIGNAL(finished(int)), this, SLOT(finished()));
    connect(machineProcess, SIGNAL(started()), this, SLOT(started()));
    connect(machineProcess, SIGNAL(suspending(QString)), this, SLOT(suspending()));
    connect(machineProcess, SIGNAL(suspended(QString)), this, SLOT(suspended()));
    connect(machineProcess, SIGNAL(resuming(QString)), this, SLOT(resuming()));
    connect(machineProcess, SIGNAL(resumed(QString)), this, SLOT(resumed()));
    connect(machineProcess, SIGNAL(error(QString)), this, SLOT(error(QString)));

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

    stopButton = new QPushButton(QIcon(":/images/" + iconTheme + "/stop.png"), tr("&Stop"), this);
    stopButton->setWhatsThis(tr("Stop this virtual machine"));
    stopButton->setIconSize(QSize(22, 22));
    stopButton->setEnabled(false);
    connect(stopButton, SIGNAL(clicked(bool)), this, SLOT(stop()));

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

    QLabel *cpuDescriptionLabel = new QLabel(tr("<hr>Choose the number of &virtual CPUs:"), this);
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

    QCheckBox *bridgedModeNetwork = new QCheckBox(tr("Bridged networking"));
    bridgedModeNetwork->setToolTip(tr("In this mode the virtual machine will have direct\n"
                                      "access to the host's network; This is needed to allow\n"
                                      "ICMP (ping) to work. This mode does not work with most\n"
                                      "wireless cards."));
    connect(bridgedModeNetwork, SIGNAL(stateChanged(int)), machineProcess, SLOT(bridgedModeNetwork(int)));
    QCheckBox *localBridgeModeNetwork = new QCheckBox(tr("Local bridged networking"));
    localBridgeModeNetwork->setToolTip(tr("This mode allows more advanced bridging techniques,\n"
                                          "including using the host computer as a router or\n"
                                          "restricting access to the host machine only."));
    connect(localBridgeModeNetwork, SIGNAL(stateChanged(int)), machineProcess, SLOT(localBridgeModeNetwork(int)));
    QCheckBox *sharedVlanNetwork = new QCheckBox(tr("Shared Vlan Networking"));
    sharedVlanNetwork->setToolTip(tr("This mode adds a network that is shared exclusively\n"
                                     "between virtual machines. IP based guests will default\n"
                                     "to APIPA addresses unless you run a DHCP server on\n"
                                     "one of your virtual machines."));
    connect(sharedVlanNetwork, SIGNAL(stateChanged(int)), machineProcess, SLOT(sharedVlanNetwork(int)));
    
    networkFrameLayout->addWidget(bridgedModeNetwork);
    networkFrameLayout->addWidget(localBridgeModeNetwork);
    networkFrameLayout->addWidget(sharedVlanNetwork);
    
    //FIXME: these 3 lines need to go away
    networkDescriptionLabel = new QLabel(tr("Custom Networking Options:"), this);
    networkDescriptionLabel->setWordWrap(true);
    networkFrameLayout->addWidget(networkDescriptionLabel);
    networkCustomOptionsEdit = new QLineEdit(this);
    //networkCustomOptionsEdit->setHidden(true);
    networkFrameLayout->addWidget(networkCustomOptionsEdit);
    

    connect(networkCustomOptionsEdit, SIGNAL(textChanged(const QString&)),
            machineProcess, SLOT(networkCustomOptions(const QString&)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), networkDescriptionLabel, SLOT(setEnabled(bool)));
    connect(networkCheckBox, SIGNAL(toggled(bool)), networkCustomOptionsEdit, SLOT(setEnabled(bool)));
    //network section end

    //sound section start
    soundButton = new QPushButton(QIcon(":/images/" + iconTheme + "/sound.png"), tr("&Sound"), this);
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

    QLabel *soundDescriptionLabel = new QLabel(tr("Choose whether sound support should "
                                                  "be available for this virtual machine."), this);
    soundDescriptionLabel->setWordWrap(true);

    soundCheckBox = new QCheckBox(tr("&Enable sound"), this);
    QLabel *soundSystemDescriptionLabel = new QLabel(tr("Choose whether to use ALSA or OSS for sound emulation."), this);
    soundSystemDescriptionLabel->setWordWrap(true);
    soundSystemCheckBox = new QCheckBox(tr("&Use ALSA"), this);

    connect(soundCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(sound(int)));
    soundFrameLayout->addWidget(soundCheckBox);
    
    connect(soundSystemCheckBox, SIGNAL(stateChanged(int)), machineProcess, SLOT(soundSystem(int)));
    
    soundFrameLayout->addWidget(soundDescriptionLabel);
    soundFrameLayout->addWidget(soundCheckBox);
    soundFrameLayout->addWidget(soundSystemDescriptionLabel);
    soundFrameLayout->addWidget(soundSystemCheckBox);
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
                                                  "host and virtual system. This option depends on the "
                                                  "operating system. It is for example not supported by text based "
                                                  "systems. <strong>Attention:</strong> "
                                                  "This option may reduce the system performance. "), this);
    mouseDescriptionLabel->setWordWrap(true);
    otherFrameLayout->addWidget(mouseDescriptionLabel);

    mouseCheckBox = new QCheckBox(tr("Enable seamless mo&use"), this);
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
    buttonsLayout->addWidget(snapshotCheckBox);
    buttonsLayout->addWidget(notesLabel);
    buttonsLayout->addWidget(notesTextEdit);
    buttonsLayout->addWidget(devicesLabel);
    buttonsLayout->addLayout(devicesLayout);
    buttonsLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    read();

    //read first the name, otherwise the name of the main tab changes
    connect(machineNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(QString)));

    //save it after each change
    connect(machineNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(snapshotCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(notesTextEdit, SIGNAL(textChanged()), this, SLOT(write()));
    connect(memorySpinBox, SIGNAL(valueChanged(int)), this, SLOT(write()));
    connect(hddPathLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(cdromLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(cdBootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(floppyLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(floppyBootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(networkCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(networkCustomOptionsEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(soundCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(soundSystemCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(mouseCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(timeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(virtualizationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
    connect(cpuSpinBox, SIGNAL(valueChanged(int)), this, SLOT(write()));
    connect(additionalOptionsEdit, SIGNAL(textChanged(const QString&)), this, SLOT(write()));
    connect(additionalOptionsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(write()));
}

//the functionality in here really should be abstracted into another class, like MachineImage
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

//the functionality in here really should be abstracted into another class, like MachineImage
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

//the functionality in here really should be abstracted into another class, like MachineImage
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
#ifdef Q_OS_WIN32
    QMessageBox::warning(window(), tr("QtEmu"),
                                   tr("This function is not available under Windows due to the missing function "
                                      "of QEMU under Windows. It will probably be fixed in a later version."));
    return;
#endif
    QString newCdPath = QFileDialog::getExistingDirectory(this, tr("Select a CD ROM Drive"),
                                                          myMachinesPath);
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
    QFile file(xmlFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(window(), tr("QtEmu"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(xmlFileName)
                             .arg(file.errorString()));
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(&file, true, &errorStr, &errorLine,
                                &errorColumn))
    {
        QMessageBox::warning(window(), tr("QtEmu"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return false;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "qtemu")
    {
        QMessageBox::warning(window(), tr("QtEmu"),
                                 tr("The file is not a QtEmu file."));
        return false;
    }

    else if (root.hasAttribute("version") && root.attribute("version") != "1.0")
    {
        QMessageBox::warning(window(), tr("QtEmu"),
                                 tr("The file is not a QtEmu version 1.0 file."));
        return false;
    }


    QDomElement child = root.firstChildElement("machine");

    //init values
    snapshotCheckBox->setChecked(false);
    cdromLineEdit->setText(QString());
    cdBootCheckBox->setChecked(false);
    floppyLineEdit->setText(QString());
    floppyBootCheckBox->setChecked(false);
    networkCheckBox->setChecked(true);
    soundCheckBox->setChecked(true);
    soundCheckBox->setChecked(false);
    networkCustomOptionsEdit->setText(QString());
    additionalOptionsCheckBox->setChecked(true);
    additionalOptionsEdit->setText(QString());
    mouseCheckBox->setChecked(true);
    timeCheckBox->setChecked(true);
    virtualizationCheckBox->setChecked(true);
    cpuSpinBox->setValue(1);

    machineNameEdit->setText(child.firstChildElement("name").text());
    snapshotCheckBox->setChecked(child.firstChildElement("snapshot").text() == "true");
    notesTextEdit->setPlainText(child.firstChildElement("notes").text());
    hddPathLineEdit->setText(child.firstChildElement("hdd").text());
    memorySlider->setValue(child.firstChildElement("memory").text().toInt());
    cdromLineEdit->setText(child.firstChildElement("cdrom").text());
    cdBootCheckBox->setChecked(child.firstChildElement("bootFromCd").text() == "true");
    floppyLineEdit->setText(child.firstChildElement("floppy").text());
    floppyBootCheckBox->setChecked(child.firstChildElement("bootFromFloppy").text() == "true");
    networkCheckBox->setChecked(child.firstChildElement("network").text() == "true");
    soundCheckBox->setChecked(child.firstChildElement("sound").text() == "true");
    soundSystemCheckBox->setChecked(child.firstChildElement("soundSystem").text() == "alsa");
    networkCustomOptionsEdit->setText(child.firstChildElement("networkCustomOptions").text());
    mouseCheckBox->setChecked(child.firstChildElement("mouse").text() == "true");
    timeCheckBox->setChecked(child.firstChildElement("time").text() == "true");
    virtualizationCheckBox->setChecked(child.firstChildElement("virtualization").text() == "true");
    cpuSpinBox->setValue(child.firstChildElement("cpu").text().toInt());
    additionalOptionsEdit->setText(child.firstChildElement("additionalOptions").text());
    additionalOptionsCheckBox->setChecked(child.firstChildElement("useAdditionalOptions").text() == "true");

    return true;
}

void MachineTab::changeValue(const QString &name, const QString &value)
{
    QDomElement element = domDocument.documentElement();
    QDomElement child = element.firstChildElement("machine");

    if (!child.isNull())
    {
        QDomElement oldElement = child.firstChildElement(name);
        if (oldElement == QDomElement())
        {
            oldElement = domDocument.createElement(name);
            child.appendChild(oldElement);
        }

        QDomElement newElement = domDocument.createElement(name);
        QDomText newText = domDocument.createTextNode(value);
        newElement.appendChild(newText);
        child.replaceChild(newElement, oldElement);
    }
}

bool MachineTab::write()
{
    changeValue("name", machineNameEdit->text());
    changeValue("snapshot", snapshotCheckBox->isChecked() ? "true" : "false");
    changeValue("notes", notesTextEdit->toPlainText());
    changeValue("hdd", hddPathLineEdit->text());
    changeValue("memory", QString::number(memorySlider->value()));
    changeValue("cdrom", cdromLineEdit->text());
    changeValue("bootFromCd", cdBootCheckBox->isChecked() ? "true" : "false");
    changeValue("floppy", floppyLineEdit->text());
    changeValue("bootFromFloppy", floppyBootCheckBox->isChecked() ? "true" : "false");
    changeValue("network", networkCheckBox->isChecked() ? "true" : "false");
    changeValue("sound", soundCheckBox->isChecked() ? "true" : "false");
    changeValue("soundSystem", soundSystemCheckBox->isChecked() ? "alsa" : "oss");
    changeValue("networkCustomOptions", networkCustomOptionsEdit->text());
    changeValue("mouse", mouseCheckBox->isChecked() ? "true" : "false");
    changeValue("time", timeCheckBox->isChecked() ? "true" : "false");
    changeValue("virtualization", virtualizationCheckBox->isChecked() ? "true" : "false");
    changeValue("cpu", QString::number(cpuSpinBox->value()));
    changeValue("additionalOptions", additionalOptionsEdit->text());
    changeValue("useAdditionalOptions", additionalOptionsCheckBox->isChecked() ? "true" : "false");

    QFile file(xmlFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("QtEmu"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(xmlFileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    domDocument.save(out, 4);
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
    pauseButton->setEnabled(true);
    resumeButton->setHidden(true);
    hddUpgradeButton->setEnabled(false);
    suspendButton->setHidden(false);
    machineProcess->start();
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
    resumeButton->setEnabled(true);
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

void MachineTab::stop()
{
    QMessageBox msgBox;
    msgBox.setText(tr("This will tell the current machine to power down. Are you sure?<br />"
                             "If the virtual machine Operating System is ACPI or APM aware, it will power down gracefully.<br />"
                             "If the machine is unresponsive, you can choose a forced shutdown. Doing this may cause damage to the disk image."));
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QPushButton *shutdownButton = msgBox.addButton(tr("Shutdown"), QMessageBox::ActionRole);
    QPushButton *forceShutdownButton = msgBox.addButton(tr("Force Shutdown"), QMessageBox::DestructiveRole);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    if (msgBox.clickedButton()==shutdownButton)
        machineProcess->stop();
    else if (msgBox.clickedButton()==forceShutdownButton)
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
}

void MachineTab::started()
{
    if(snapshotCheckBox->isChecked())
    {
       snapshotCheckBox->setText(snapshotCheckBox->text() + tr("(uncheck to commit changes)"));
    }
}

void MachineTab::error(const QString & errorMsg)
{
    QMessageBox::critical(this, tr("QtEmu Error"), tr("An error has occurred in qemu relating to something you were doing. The error is:<br />") + errorMsg,QMessageBox::Ok);
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
//#ifdef Q_OS_WIN32
//    QMessageBox::warning(window(), tr("QtEmu"),
//                                   tr("This function is not available under Windows due to the missing function "
//                                      "of QEMU under Windows. It will probably be fixed in a later version."));
//    return;
//#endif
    QString newSmbPath = QFileDialog::getExistingDirectory(this, tr("Select a folder to use as a Virtual Network Drive"),
                                                          myMachinesPath);
    if (!newSmbPath.isEmpty())
        smbFolderEdit->setText(newSmbPath);
}

void MachineTab::network(const int value)
{
}
