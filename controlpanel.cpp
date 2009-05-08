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

/****************************************************************************
** C++ Implementation: controlpanel
**
** Description: 
**
****************************************************************************/
#include "controlpanel.h"

#include "machineconfigobject.h"
#include "machinetab.h"
#include "machineprocess.h"
#include "settingstab.h"
#include "machineview.h"
#include "usbpage.h"
#include "usbmodel.h"

#include <QFileDialog>
#include <QStandardItemModel>

ControlPanel::ControlPanel(MachineTab *parent)
 : QWidget(parent)
 , parent(parent)
{
    setupUi(this);
    config = parent->machineConfigObject;

    //load current drives
    foreach(OptDevice device, QtEmuEnvironment::getHal()->opticalList())
        cdCombo->addItem(device.name, device.device);

    registerObjects();
    makeConnections();

    QPalette listPalette = usbView->palette();
    QColor transparent = QColor();
    transparent.setAlpha(0);
    listPalette.setColor(QPalette::Base, transparent);
    usbView->setPalette(listPalette);
}


ControlPanel::~ControlPanel()
{
}

void ControlPanel::makeConnections()
{
    //navigation connections
    connect(mediaButton, SIGNAL(clicked()), this, SLOT(mediaActivate()));
    //connect(optionButton, SIGNAL(clicked()), this, SLOT(optionActivate()));
    connect(displayButton, SIGNAL(clicked()), this, SLOT(displayActivate()));
    connect(usbButton, SIGNAL(clicked()), this, SLOT(usbActivate()));

    //action connections
    connect(cdReloadButton, SIGNAL(clicked()), parent->machineProcess, SLOT(changeCdrom()));
    connect(floppyReloadButton, SIGNAL(clicked()), parent->machineProcess, SLOT(changeFloppy()));
    connect(cdImageButton, SIGNAL(clicked()), parent->settingsTab, SLOT(setNewCdImagePath()));
    connect(floppyImageButton, SIGNAL(clicked()), parent->settingsTab, SLOT(setNewFloppyImagePath()));

    connect(fullscreenButton, SIGNAL(toggled(bool)), parent->machineView, SLOT(fullscreen(bool)));
    connect(parent->machineView, SIGNAL(fullscreenToggled(bool)), fullscreenButton, SLOT(setChecked(bool)));

    connect(screenshotButton, SIGNAL(clicked()), this, SLOT(saveScreenshot()));

    //state connections
    connect(parent->machineProcess, SIGNAL(started()), this, SLOT(running()));
    connect(parent->machineProcess, SIGNAL(finished()), this, SLOT(stopped()));

    //connections for optical drive detection
    connect(QtEmuEnvironment::getHal(), SIGNAL(opticalAdded(QString,QString)),this,SLOT(optAdded(QString,QString)));
    connect(QtEmuEnvironment::getHal(), SIGNAL(opticalRemoved(QString,QString)),this,SLOT(optRemoved(QString,QString)));
}

void ControlPanel::mediaActivate()
{
    controlStack->setCurrentIndex(0);
}

void ControlPanel::displayActivate()
{
    controlStack->setCurrentIndex(2);
}

void ControlPanel::usbActivate()
{
    controlStack->setCurrentIndex(1);
}

void ControlPanel::registerObjects()
{
    config->registerObject(cdCombo, "cdrom");
    config->registerObject(floppyCombo, "floppy");
    config->registerObject(mouseButton, "mouse");
    config->registerObject(scaleButton, "scaleEmbeddedDisplay");
    config->registerObject(addDevices, "autoAddDevices");

    usbFrame->setProperty("enableDisable", true);
    config->registerObject(usbFrame, "usbSupport");

    //connect the usb view to the model.
    usbView->setModel(parent->settingsTab->getUsbPage()->getModel());
}

void ControlPanel::saveScreenshot()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save a Screenshot"),
                                                     QString(),
                                                     tr("Pictures")+" (*.ppm)");
    if(!fileName.endsWith(".ppm"))
        fileName = fileName + ".ppm";

    parent->machineProcess->write(QString("screendump " + fileName).toAscii() + '\n');
}

void ControlPanel::running()
{
    fullscreenButton->setEnabled(true);
    screenshotButton->setEnabled(true);
}

void ControlPanel::stopped()
{
    fullscreenButton->setEnabled(false);
    screenshotButton->setEnabled(false);
}

void ControlPanel::optionChanged(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value)
{
    if(optionName == "autoAddDevices")
    {
        usbView->setEnabled(config->getOption("autoAddDevices", true).toBool());
    }
}

void ControlPanel::optAdded(QString devName, QString devPath)
{
    cdCombo->addItem(devName, devPath);
}

void ControlPanel::optRemoved(QString devName, QString devPath)
{
    if(cdCombo->itemData(cdCombo->currentIndex()).toString() == devPath)
    {
        parent->machineProcess->changeCdrom();
    }
    cdCombo->removeItem(cdCombo->findData(devPath));
}
