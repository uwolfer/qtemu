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

#include <QFileDialog>

ControlPanel::ControlPanel(MachineTab *parent)
 : QWidget(parent)
 , parent(parent)
{
    setupUi(this);
    config = parent->machineConfigObject;
    registerObjects();
    makeConnections();
}


ControlPanel::~ControlPanel()
{
}

void ControlPanel::makeConnections()
{
    //navigation connections
    connect(mediaButton, SIGNAL(clicked()), this, SLOT(mediaActivate()));
    connect(optionButton, SIGNAL(clicked()), this, SLOT(optionActivate()));
    connect(displayButton, SIGNAL(clicked()), this, SLOT(displayActivate()));

    //action connections
    connect(cdReloadButton, SIGNAL(clicked()), parent->machineProcess, SLOT(changeCdrom()));
    connect(floppyReloadButton, SIGNAL(clicked()), parent->machineProcess, SLOT(changeFloppy()));
    connect(cdImageButton, SIGNAL(clicked()), parent->settingsTab, SLOT(setNewCdImagePath()));
    connect(floppyImageButton, SIGNAL(clicked()), parent->settingsTab, SLOT(setNewFloppyImagePath()));

    connect(fullscreenButton, SIGNAL(toggled(bool)), parent->machineView, SLOT(fullscreen(bool)));
    connect(parent->machineView, SIGNAL(fullscreenToggled(bool)), fullscreenButton, SLOT(setChecked(bool)));

    connect(screenshotButton, SIGNAL(clicked()), this, SLOT(saveScreenshot()));
}

void ControlPanel::mediaActivate()
{
    controlStack->setCurrentIndex(0);
}

void ControlPanel::optionActivate()
{
    controlStack->setCurrentIndex(1);
}

void ControlPanel::displayActivate()
{
    controlStack->setCurrentIndex(2);
}

void ControlPanel::registerObjects()
{
    config->registerObject(cdCombo, "cdrom");
    config->registerObject(floppyCombo, "floppy");
    config->registerObject(mouseButton, "mouse");
    config->registerObject(scaleButton, "scaleEmbeddedDisplay");
    
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
