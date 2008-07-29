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
**
** C++ Implementation: settingstab
**
** Description: 
**
****************************************************************************/

#include "settingstab.h"
#include "machineconfigobject.h"
#include "helpwindow.h"

#include <QIcon>


SettingsTab::SettingsTab(MachineConfigObject *config, QWidget *parent)
 : QFrame(parent)
{
    setupUi(this);

    this->config = config;
    //icons can't change based on a setting in a ui file...
    void setIcons();
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
    config->registerObject(networkCheck, "network", QVariant(true));
    config->registerObject(networkEdit, "networkCustomOptions");
}

void SettingsTab::changeHelpTopic(int page)
{
    //QString helpFile = ;
    QUrl helpFile = QUrl(HelpWindow::getHelpLocation().toString() + "dynamic/" + settingsStack->currentWidget()->property("helpFile").toString());
    helpView->load(helpFile);

}

void SettingsTab::setIcons()
{

}

void SettingsTab::setupHelp()
{
    //set up the help browser
    helpArea->hide();
    changeHelpTopic(0);
    connect(settingsStack, SIGNAL(currentChanged(int)), this, SLOT(changeHelpTopic(int)));
}

