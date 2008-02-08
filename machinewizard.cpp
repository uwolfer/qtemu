/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
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

#include "machinewizard.h"

#include "config.h"

#include <QProcess>
#include <QMessageBox>
#include <QFile>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QIcon>
#include <QComboBox>
#include <QTextStream>
#include <QSettings>
#include <QPushButton>
#include <QFileDialog>
#include <QDomElement>

MachineWizard::MachineWizard(const QString &myMachinesPathParent, QWidget *parent)
    : Wizard(parent)
{
    myMachinesPath = myMachinesPathParent;
    chooseSystemPage = new ChooseSystemPage(this);
    locationPage = new LocationPage(this);
    imagePage = new ImagePage(this);

    setFirstPage(chooseSystemPage);

    setWindowTitle(tr("Create a new Machine"));
    resize(400, 350);
}

QString MachineWizard::newMachine(const QString &myMachinesPath, QWidget *parent)
{
    MachineWizard wizard(myMachinesPath, parent);

    QLineEdit *pathLineEdit = qobject_cast<QLineEdit *>(wizard.locationPage->pathLineEdit);
    QLineEdit *nameLineEdit = qobject_cast<QLineEdit *>(wizard.locationPage->nameLineEdit);

    QString result;
    bool accepted = (wizard.exec() == QDialog::Accepted);
    if (accepted)
        result = pathLineEdit->text()+'/'+nameLineEdit->text().replace(' ', '_')+".qte";
    else
        result = QString();

    return result;
}

ChooseSystemPage::ChooseSystemPage(MachineWizard *wizard)
    : MachineWizardPage(wizard)
{
    comboSystem = new QComboBox;
    comboSystem->addItem(tr("Select a System..."));
    comboSystem->addItem(tr("Linux"));
    comboSystem->addItem(tr("Windows 98"));
    comboSystem->addItem(tr("Windows 2000"));
    comboSystem->addItem(tr("Windows XP"));
    comboSystem->addItem(tr("ReactOS"));
    comboSystem->addItem(tr("Other"));

    connect(comboSystem, SIGNAL(activated(int)), this, SIGNAL(completeStateChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(comboSystem);
    layout->addStretch();
    setLayout(layout);
}

void ChooseSystemPage::updateTitle()
{
    wizard->setTitle(tr("Select the operating system you want to install"));
}

void ChooseSystemPage::resetPage()
{
}

WizardPage *ChooseSystemPage::nextPage()
{
    wizard->osName = comboSystem->currentText();
    return wizard->locationPage;
}

bool ChooseSystemPage::isComplete()
{
    return !(comboSystem->currentText()==tr("Select a System..."));
}


LocationPage::LocationPage(MachineWizard *wizard)
    : MachineWizardPage(wizard)
{
    nameLabel = new QLabel(tr("&Name:"));
    nameLineEdit = new QLineEdit;
    nameLabel->setBuddy(nameLineEdit);
    connect(nameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(privateSlot()));

    pathLabel = new QLabel(tr("&Path:"));
    pathLineEdit = new QLineEdit;
    privateSlot();
    pathLabel->setBuddy(pathLineEdit);

    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();

    QPushButton *pathSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/open.png"), QString(), this);
    connect(pathSelectButton, SIGNAL(clicked()), this, SLOT(setNewPath()));

    QHBoxLayout *pathLayout = new QHBoxLayout;
    pathLayout->addWidget(pathLineEdit);
    pathLayout->addWidget(pathSelectButton);

    connect(nameLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(completeStateChanged()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(nameLabel, 1, 0);
    layout->addWidget(nameLineEdit, 1, 1);
    layout->addWidget(pathLabel, 2, 0);
    layout->addLayout(pathLayout, 2, 1);
    layout->setRowStretch(3, 1);
    setLayout(layout);
}

void LocationPage::updateTitle()
{
    wizard->setTitle(tr("Choose name and location for the new machine"));
}

void LocationPage::privateSlot()
{
    pathLineEdit->setText(wizard->myMachinesPath+
                          '/'+nameLineEdit->text().replace(' ', '_'));
}

void LocationPage::setNewPath()
{
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Select a folder for saving the hard disk image"),
                                                        wizard->myMachinesPath);
    if (!newPath.isEmpty())
        pathLineEdit->setText(newPath);
}

void LocationPage::resetPage()
{
    nameLineEdit->setText(wizard->osName);
}

WizardPage *LocationPage::nextPage()
{
    wizard->osNameUser = nameLineEdit->text();
    wizard->osPathUser = pathLineEdit->text();
    return wizard->imagePage;
}

bool LocationPage::isComplete()
{
    return !(nameLineEdit->text().isEmpty());
}


ImagePage::ImagePage(MachineWizard *wizard)
    : MachineWizardPage(wizard)
{
    sizeLabel = new QLabel(tr("&Disk image size:"));
    sizeSpinBox = new QDoubleSpinBox;
    sizeSpinBox->setFixedSize(sizeSpinBox->sizeHint());
    sizeLabel->setBuddy(sizeSpinBox);
    sizeGbLabel = new QLabel(tr("GB"));
    sizeGbLabel->setFixedSize(sizeGbLabel->sizeHint());
    sizeGbLabel->setBuddy(sizeSpinBox);
    connect(sizeSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(completeStateChanged()));

    formatLabel = new QLabel(tr("Disk image format:"));
    formatComboBox = new QComboBox;
    formatComboBox->addItem(tr("Native image (qcow)"));
    formatComboBox->addItem(tr("Raw image (img)"));
    formatComboBox->addItem(tr("VMWare image (vmdk)"));
    formatInfoLabel = new QLabel(tr("The native image format enables<br>"
                                     "suspend/resume features, all other formats<br>"
                                     "lack suspend/resume. Use \"Native image (qcow)\"<br>"
                                     "unless you know what you are doing."));
    formatLabel->setBuddy(formatComboBox);
    connect(formatComboBox, SIGNAL(activated(int)), this, SIGNAL(completeStateChanged()));

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(sizeSpinBox);
    sizeLayout->addWidget(sizeGbLabel);

    QHBoxLayout *formatLayout = new QHBoxLayout;
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatComboBox);

    layout->addLayout(sizeLayout);
    layout->addLayout(formatLayout);
    layout->addStretch();
    layout->addWidget(formatInfoLabel);
    layout->addStretch();
    setLayout(layout);

    connect(wizard, SIGNAL(finished()), this, SLOT(privateSlot()));
}

void ImagePage::updateTitle()
{
    wizard->setTitle(tr("Specify disk image details"));
}

void ImagePage::resetPage()
{
    sizeSpinBox->setValue(0);
}

bool ImagePage::isComplete()
{
    return (sizeSpinBox->value()!=0);
}

void ImagePage::privateSlot()
{
    QString osName = wizard->osNameUser;
    QString osPath = wizard->osPathUser;

    dir = new QDir();
    dir->mkpath(osPath);

    QDomDocument domDocument("qtemu");
    QDomProcessingInstruction process = domDocument.createProcessingInstruction(
                              "xml", "version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.appendChild(process);

    QDomElement root = domDocument.createElement("qtemu");
    root.setAttribute("version", "1.0");
    domDocument.appendChild(root);

    QDomElement machine = domDocument.createElement("machine");
    root.appendChild(machine);

    QDomElement domElement;
    QDomText domText;

    domElement = domDocument.createElement("name");
    machine.appendChild(domElement);
    domText = domDocument.createTextNode(osName);
    domElement.appendChild(domText);

    domElement = domDocument.createElement("hdd");
    machine.appendChild(domElement);
    if(formatComboBox->currentIndex()==0)
        domText = domDocument.createTextNode(osPath+'/'+osName.replace(' ', '_')+".qcow");
    else if(formatComboBox->currentIndex()==1)
    domText = domDocument.createTextNode(osPath+'/'+osName.replace(' ', '_')+".img");
    else
        domText = domDocument.createTextNode(osPath+'/'+osName.replace(' ', '_')+".vmdk");
    
    domElement.appendChild(domText);

    domElement = domDocument.createElement("memory");
    machine.appendChild(domElement);
    domText = domDocument.createTextNode("128");
    domElement.appendChild(domText);

    domElement = domDocument.createElement("notes");
    machine.appendChild(domElement);
    domText = domDocument.createTextNode(tr("Click here to write down some notes "
                                            "about this machine."));
    domElement.appendChild(domText);

    domElement = domDocument.createElement("snapshot");
    machine.appendChild(domElement);
#ifdef DEVELOPER
    domText = domDocument.createTextNode("true");
#else
    domText = domDocument.createTextNode("false");
#endif
    domElement.appendChild(domText);

    QFile file(osPath+'/'+osName.replace(' ', '_')+".qte");
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::critical(this, tr("Error"), tr("Image NOT created!"));
        return;
    }

    QTextStream out(&file);
    domDocument.save(out, 4);

    imageCreateProcess = new QProcess(this);
    imageCreateProcess->setWorkingDirectory(osPath);
    QStringList arguments;

    if(formatComboBox->currentIndex()==0)
    {
    arguments << "create" << "-f" << "qcow2"
              << osName.replace(' ', '_')+".qcow";
    }
    else if(formatComboBox->currentIndex()==1)
    {
    arguments << "create" << "-f" << "raw"
              << osName.replace(' ', '_')+".img";
    }
    else
    {
    arguments << "create" << "-f" << "vmdk"
              << osName.replace(' ', '_')+".vmdk";
    }

    arguments << QString::number(sizeSpinBox->value()*1000)+'M';
#ifndef Q_OS_WIN32
    imageCreateProcess->start("qemu-img", arguments);
#elif defined(Q_OS_WIN32)
    imageCreateProcess->start("qemu/qemu-img.exe", arguments);
#endif
    QMessageBox::information(this, tr("Finished"), tr("Image created"));
}
