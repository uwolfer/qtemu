/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop@gmail.com>
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

#include "settingstab.h"
#include "machineconfigobject.h"
#include "helpwindow.h"

#include "networkpage.h"

#include <QIcon>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QVBoxLayout>


SettingsTab::SettingsTab(MachineConfigObject *config, MachineTab *parent)
 : QFrame(parent)
 ,config(config)
 ,parent(parent)
{
    getSettings();
    //add the ui elements
    setupUi(this);
    netPage = new NetworkPage(config, this);
    networkPage->setLayout(new QVBoxLayout());
    networkPage->layout()->addWidget(netPage);

    setupConnections();

    //register all the widgets with their associated options
    registerWidgets();
    setupHelp();
}


SettingsTab::~SettingsTab()
{
}




void SettingsTab::registerWidgets()
{
    config->registerObject(cpuSpinBox, "cpu", QVariant(1));
    config->registerObject(memorySpinBox, "memory");
    config->registerObject(virtCheckBox, "virtualization", QVariant(false));
    config->registerObject(hdImage, "hdd");
    config->registerObject(cdImage, "cdrom");
    config->registerObject(floppyImage, "floppy");
    config->registerObject(cdBootCheck, "bootFromCd", QVariant(false));
    config->registerObject(floppyBootCheck, "bootFromFloppy", QVariant(false));
    config->registerObject(soundCheck, "sound", QVariant(false));
    config->registerObject(soundCombo, "soundSystem", QVariant("oss"));
    config->registerObject(timeCheck, "time", QVariant(true));
    config->registerObject(embedCheck, "embeddedDisplay", QVariant(true));
    config->registerObject(scaleCheck, "scaleEmbeddedDisplay", QVariant(true));
    config->registerObject(portBox, "vncPort");
    config->registerObject(hostEdit, "vncHost", QVariant("localhost"));
    config->registerObject(tcpRadio, "vncTransport", QVariant("tcp"));
    config->registerObject(fileRadio, "vncTransport");
    config->registerObject(additionalCheck, "useAdditionalOptions", QVariant(false));
    config->registerObject(additionalEdit, "additionalOptions");
    config->registerObject(beforeCheck, "enableExecBefore", QVariant(false));
    config->registerObject(beforeEdit, "execBefore");
    config->registerObject(afterCheck, "enableExecAfter", QVariant(false));
    config->registerObject(afterEdit, "execAfter");
    config->registerObject(osCheck, "operatingSystem", QVariant("Other"));
    config->registerObject(hiResCheck, "hiRes", QVariant(true));
    config->registerObject(mouseCheck, "mouse", QVariant(true));
    config->registerObject(acpiCheck, "acpi", QVariant(true));
    config->registerObject(hddAccelCheck, "hddVirtio", QVariant(false));
    config->registerObject(usbCheck, "usbSupport", QVariant(true));
}

void SettingsTab::changeHelpTopic()
{
    //QString helpFile = ;
    QUrl helpFile(HelpWindow::getHelpLocation().toString() + "dynamic/" + settingsStack->currentWidget()->property("helpFile").toString());
    helpView->load(helpFile);
}

void SettingsTab::setupHelp()
{
    //set up the help browser
    helpArea->hide();
    changeHelpTopic();
    connect(settingsStack, SIGNAL(currentChanged(int)), this, SLOT(changeHelpTopic()));
}


void SettingsTab::setupConnections()
{
    connect(hdSelectButton, SIGNAL(clicked()), this, SLOT(setNewHddPath()));
    connect(cdSelectButton, SIGNAL(clicked()), this, SLOT(setNewCdImagePath()));
    connect(floppySelectButton, SIGNAL(clicked()), this, SLOT(setNewFloppyImagePath()));
    connect(upgradeButton, SIGNAL(clicked()), this, SLOT(confirmUpgrade()));
}

//various file select dialogs
void SettingsTab::setNewHddPath()
{
    QString newHddPath = QFileDialog::getOpenFileName(this, tr("Select a QtEmu hard disk image"),
                                              config->getOption("hdd", myMachinesPath).toString(),
                                              tr("QtEmu hard disk images")+" (*.img *.qcow *.vmdk *.hdd *.vpc)");
    if (!newHddPath.isEmpty())
        config->setOption("hdd", newHddPath);
}
void SettingsTab::setNewCdImagePath()
{
    QString newCdPath = QFileDialog::getOpenFileName(this, tr("Select a CD Image"),
                                                     config->getOption("cdrom", myMachinesPath).toString(),
                                                     tr("CD ROM images")+" (*.iso *.img)");
    if (!newCdPath.isEmpty())
        config->setOption("cdrom", newCdPath);
}
void SettingsTab::setNewFloppyImagePath()
{
    QString newFloppyPath = QFileDialog::getOpenFileName(this, tr("Select a Floppy Disk Image"),
                                                         config->getOption("floppy", myMachinesPath).toString(),
                                                         tr("Floppy disk images")+" (*.iso *.img)");
    if (!newFloppyPath.isEmpty())
        config->setOption("floppy", newFloppyPath);
}
//end file select dialogs

//warning dialogs
void SettingsTab::confirmUpgrade()
{
    if (QMessageBox::question(this, tr("Upgrade Confirmation"),
                              tr("This will upgrade your Hard Disk image to the qcow format.<br />This enables more advanced features such as suspend/resume on all host operating systems and image compression on Windows hosts.<br />Your old image will remain intact, so if you want to revert afterwards you may do so."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
      == QMessageBox::Yes)
    {
        emit upgradeHdd();
    }
}
//end warning dialogs

//load program wide settings
void SettingsTab::getSettings()
{
    QSettings settings("QtEmu", "QtEmu");
    myMachinesPath = settings.value("machinesPath", QString(QDir::homePath()+'/'+tr("MyMachines"))).toString();
}

void SettingsTab::setVirtSize(qint64 size)
{
    QString sizeS;
    float sizeK = size/1024.0;
    float sizeM = sizeK/1024.0;
    float sizeG = sizeM/1024.0;
    if(sizeM<1)
        sizeS = QString::number((int)sizeK) + "." + QString::number(((int)size)%1024) + " Kilobyte" + ((sizeK<2)?"":"s");
    else if(sizeG<1)
        sizeS = QString::number((int)sizeM) + "." + QString::number(((int)sizeK)%1024) + " Megabyte" + ((sizeM<2)?"":"s");
    else
        sizeS = QString::number((int)sizeG) + "." + QString::number(((int)sizeM)%1024) + " Gigabyte" + ((sizeG<2)?"":"s");    virtSizeLabel->setText(sizeS);
}

void SettingsTab::setPhySize(qint64 size)
{
    QString sizeS;
    float sizeK = size/1024.0;
    float sizeM = sizeK/1024.0;
    float sizeG = sizeM/1024.0;
    if(sizeM<1)
        sizeS = QString::number((int)sizeK) + "." + QString::number(((int)size)%1024) + " Kilobyte" + ((sizeK<2)?"":"s");
    else if(sizeG<1)
        sizeS = QString::number((int)sizeM) + "." + QString::number(((int)sizeK)%1024) + " Megabyte" + ((sizeM<2)?"":"s");
    else
        sizeS = QString::number((int)sizeG) + "." + QString::number(((int)sizeM)%1024) + " Gigabyte" + ((sizeG<2)?"":"s");
    phySizeLabel->setText(sizeS);
}

void SettingsTab::getDrives()
{
    //TODO:set a list of removable drives to be chosen by the dropdown menu for optical media / floppy drives. on linux this must be obtained through dbus, but on windows it can be gotten through Qt itself.
    //qDebug(QDir::drives());
}
